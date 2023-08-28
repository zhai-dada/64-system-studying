#include "disk.h"
#include "fat32.h"
#include "vfs.h"
#include "printk.h"
#include "stdio.h"
#include "fcntl.h"
#include "errno.h"

#define FAT32_START_SECTOR 616448 // 我的u盘的FAT32分区起始扇区

unsigned int readfatentry(struct FAT32_sb_info *fsbi, unsigned int fat_entry)
{
    unsigned int buffer[128];
    memset(buffer, 0, 512);
    disk_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + (fat_entry >> 7), 1, (unsigned char *)buffer);
    return buffer[fat_entry & 0x7f] & 0x0fffffff;
}
unsigned long writefatentry(struct FAT32_sb_info *fsbi, unsigned int fat_entry, unsigned int value)
{
    unsigned int buffer[128];
    memset(buffer, 0, 512);
    disk_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + (fat_entry >> 7), 1, (unsigned char *)buffer);
    int i = 0;
    buffer[fat_entry & 0x7f] = (buffer[fat_entry & 0x7f] & 0xf0000000) | (value & 0x0fffffff);
    for (i = 0; i < fsbi->NumFATs; i++)
    {
        disk_device_operation.transfer(ATA_WRITE_CMD, fsbi->FAT1_firstsector + fsbi->sector_per_FAT * i + (fat_entry >> 7), 1, (unsigned char *)buffer);
    }
    return 1;
}

long FAT32_open(struct index_node *inode, struct file *filp)
{
    return 1;
}
long FAT32_close(struct index_node *inode, struct file *filp)
{
    return 1;
}
long FAT32_read(struct file *filp, char *buf, unsigned long count, long *position)
{
    struct FAT32_inode_info *finode = filp->dentry->dir_inode->private_index_info;
    struct FAT32_sb_info *fsbi = filp->dentry->dir_inode->sb->private_sb_info;

    unsigned long cluster = finode->first_cluster;
    unsigned long sector = 0;
    int i, length = 0;
    long retval = 0;
    int index = *position / fsbi->bytes_per_cluster;
    long offset = *position % fsbi->bytes_per_cluster;
    char *buffer = (char *)kmalloc(fsbi->bytes_per_cluster, 0);

    if (!cluster)
    {
        return -EFAULT;
    }
    for (i = 0; i < index; i++)
    {
        cluster = readfatentry(fsbi, cluster);
    }
    if (*position + count > filp->dentry->dir_inode->file_size)
    {
        index = count = filp->dentry->dir_inode->file_size - *position;
    }
    else
    {
        index = count;
    }
    color_printk(GREEN, BLACK, "FAT32_read first_cluster:%d,size:%d,preempt_count:%d\n", finode->first_cluster, filp->dentry->dir_inode->file_size, current->preempt_count);

    do
    {
        memset(buffer, 0, fsbi->bytes_per_cluster);
        sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;
        if (!disk_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
        {
            color_printk(RED, BLACK, "FAT32 FS(read) read disk ERROR!!!!!!!!!!\n");
            retval = -EIO;
            break;
        }

        length = (index <= (fsbi->bytes_per_cluster - offset)) ? index : (fsbi->bytes_per_cluster - offset);

        if ((unsigned long)buf < TASK_SIZE)
        {
            copy_to_user(buffer + offset, buf, length);
        }
        else
        {
            memcpy(buffer + offset, buf, length);
        }
        index -= length;
        buf += length;
        offset -= offset;
        *position += length;
    } while (index && (cluster = readfatentry(fsbi, cluster)));

    kfree(buffer);
    if (!index)
    {
        retval = count;
    }
    return retval;
}

unsigned long FAT32_find_available_cluster(struct FAT32_sb_info *fsbi)
{
    int i = 0, j = 0;
    int fat_entry = 0;
    unsigned long sector_per_fat = fsbi->sector_per_FAT;
    unsigned int buf[128] = {0};

    for (i = 0; i < sector_per_fat; i++)
    {
        memset(buf, 0, 512);
        disk_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + i, 1, (unsigned char *)buf);

        for (j = 0; j < 128; j++)
        {
            if ((buf[j] & 0x0fffffff) == 0)
            {
                return (i << 7) + j;
            }
        }
    }
    return 0;
}

long FAT32_write(struct file *filp, char *buf, unsigned long count, long *position)
{
    struct FAT32_inode_info *finode = filp->dentry->dir_inode->private_index_info;
    struct FAT32_sb_info *fsbi = filp->dentry->dir_inode->sb->private_sb_info;

    unsigned long cluster = finode->first_cluster;
    unsigned long next_cluster = 0;
    unsigned long sector = 0;
    int i, length = 0;
    long retval = 0;
    long flags = 0;
    int index = *position / fsbi->bytes_per_cluster;
    long offset = *position % fsbi->bytes_per_cluster;
    char *buffer = (char *)kmalloc(fsbi->bytes_per_cluster, 0);

    if (!cluster)
    {
        cluster = FAT32_find_available_cluster(fsbi);
        flags = 1;
    }
    else
    {
        for (i = 0; i < index; i++)
        {
            cluster = readfatentry(fsbi, cluster);
        }
    }
    if (!cluster)
    {
        kfree(buffer);
        return -ENOSPC;
    }

    if (flags)
    {
        finode->first_cluster = cluster;
        filp->dentry->dir_inode->sb->sb_ops->write_inode(filp->dentry->dir_inode);
        writefatentry(fsbi, cluster, 0x0ffffff8);
    }

    index = count;

    do
    {
        if (!flags)
        {
            memset(buffer, 0, fsbi->bytes_per_cluster);
            sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;
            if (!disk_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
            {
                color_printk(RED, BLACK, "FAT32 FS(write) read disk ERROR!!!!!!!!!!\n");
                retval = -EIO;
                break;
            }
        }

        length = index <= fsbi->bytes_per_cluster - offset ? index : fsbi->bytes_per_cluster - offset;

        if ((unsigned long)buf < TASK_SIZE)
        {
            copy_from_user(buf, buffer + offset, length);
        }
        else
        {
            memcpy(buf, buffer + offset, length);
        }
        if (!disk_device_operation.transfer(ATA_WRITE_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
        {
            color_printk(RED, BLACK, "FAT32 FS(write) write disk ERROR!!!!!!!!!!\n");
            retval = -EIO;
            break;
        }

        index -= length;
        buf += length;
        offset -= offset;
        *position += length;

        if (index)
        {
            next_cluster = readfatentry(fsbi, cluster);
        }
        else
        {
            break;
        }
        if (next_cluster >= 0x0ffffff8)
        {
            next_cluster = FAT32_find_available_cluster(fsbi);
            if (!next_cluster)
            {
                kfree(buffer);
                return -ENOSPC;
            }

            writefatentry(fsbi, cluster, next_cluster);
            writefatentry(fsbi, next_cluster, 0x0ffffff8);
            cluster = next_cluster;
            flags = 1;
        }

    } while (index);

    if (*position > filp->dentry->dir_inode->file_size)
    {
        filp->dentry->dir_inode->file_size = *position;
        filp->dentry->dir_inode->sb->sb_ops->write_inode(filp->dentry->dir_inode);
    }

    kfree(buffer);
    if (!index)
    {
        retval = count;
    }
    return retval;
}

long FAT32_lseek(struct file *filp, long offset, long origin)
{
    struct index_node *inode = filp->dentry->dir_inode;
    long pos = 0;

    switch (origin)
    {
    case SEEK_SET:
        pos = 0 + offset;
        break;

    case SEEK_CUR:
        pos = filp->position + offset;
        break;

    case SEEK_END:
        pos = filp->dentry->dir_inode->file_size + offset;
        break;

    default:
        return -EINVAL;
        break;
    }

    if (pos < 0 || pos > filp->dentry->dir_inode->file_size)
    {
        return -EOVERFLOW;
    }
    filp->position = pos;
    return pos;
}
long FAT32_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg)
{
    return 1;
}
struct file_operations FAT32_file_ops =
    {
        .open = FAT32_open,
        .close = FAT32_close,
        .read = FAT32_read,
        .write = FAT32_write,
        .lseek = FAT32_lseek,
        .ioctl = FAT32_ioctl,
};
long FAT32_create(struct index_node *inode, struct dir_entry *dentry, int mode)
{
    return 1;
}
long FAT32_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode)
{
    return 1;
}
long FAT32_rmdir(struct index_node *inode, struct dir_entry *dentry)
{
    return 1;
}
long FAT32_rename(struct index_node *old_inode, struct dir_entry *old_dentry, struct index_node *new_inode, struct dir_entry *new_dentry)
{
    return 1;
}
long FAT32_getattr(struct dir_entry *dentry, unsigned long *attr)
{
    return 1;
}
long FAT32_setattr(struct dir_entry *dentry, unsigned long *attr)
{
    return 1;
}
struct dir_entry *FAT32_lookup(struct index_node *parent_inode, struct dir_entry *dest_dentry)
{
    struct FAT32_inode_info *finode = parent_inode->private_index_info;
    struct FAT32_sb_info *fsbi = parent_inode->sb->private_sb_info;

    unsigned int cluster = 0;
    unsigned long sector = 0;
    unsigned char *buff = NULL;
    int i = 0, j = 0, x = 0;
    struct FAT32_D *tmpdentry = NULL;
    struct FAT32_LD *tmpldentry = NULL;
    struct index_node *p = NULL;

    buff = kmalloc(fsbi->bytes_per_cluster, 0);

    cluster = finode->first_cluster;

next_cluster:
    sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;
    color_printk(BLUE, BLACK, "lookup cluster:%#010x,sector:%#018lx\n", cluster, sector);
    if (!disk_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buff))
    {
        color_printk(RED, BLACK, "FAT32 FS(lookup) read disk ERROR!!!!!!!!!!\n");
        kfree(buff);
        return NULL;
    }

    tmpdentry = (struct FAT32_D *)buff;

    for (i = 0; i < fsbi->bytes_per_cluster; i += 32, tmpdentry++)
    {
        if (tmpdentry->DIR_Attr == ATTR_LONG_NAME)
        {
            continue;
        }
        if (tmpdentry->DIR_Name[0] == 0xe5 || tmpdentry->DIR_Name[0] == 0x00 || tmpdentry->DIR_Name[0] == 0x05)
        {
            continue;
        }
        tmpldentry = (struct FAT32_LD *)tmpdentry - 1;
        j = 0;

        // long file or dir name compare
        while (tmpldentry->LDIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5)
        {
            for (x = 0; x < 5; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name1[x] == 0xffff)
                {
                    continue;
                }
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name1[x] != (unsigned short)(dest_dentry->name[j++]))
                {
                    goto continue_cmp_fail;
                }
            }
            for (x = 0; x < 6; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name2[x] == 0xffff)
                {
                    continue;
                }
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name2[x] != (unsigned short)(dest_dentry->name[j++]))
                {
                    goto continue_cmp_fail;
                }
            }
            for (x = 0; x < 2; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name3[x] == 0xffff)
                {
                    continue;
                }
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name3[x] != (unsigned short)(dest_dentry->name[j++]))
                {
                    goto continue_cmp_fail;
                }
            }

            if (j >= dest_dentry->name_length)
            {
                goto find_lookup_success;
            }

            tmpldentry--;
        }

        // short file or dir base name compare
        j = 0;
        for (x = 0; x < 8; x++)
        {
            switch (tmpdentry->DIR_Name[x])
            {
            case ' ':
                if (!(tmpdentry->DIR_Attr & ATTR_DIRECTORY))
                {
                    if (dest_dentry->name[j] == '.')
                    {
                        continue;
                    }
                    else if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }
                }
                else
                {
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else if (j == dest_dentry->name_length)
                    {
                        continue;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }
                }
            case 'A' ... 'Z':
            case 'a' ... 'z':
                if (tmpdentry->DIR_NTRes & LOWERCASE_BASE)
                {
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] + 32 == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }
                }
                else
                {
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }
                }

            case '0' ... '9':
                if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                {
                    j++;
                    break;
                }
                else
                {
                    goto continue_cmp_fail;
                }

            default:
                j++;
                break;
            }
        }
        // short file ext name compare
        if (!(tmpdentry->DIR_Attr & ATTR_DIRECTORY))
        {
            j++;
            for (x = 8; x < 11; x++)
            {
                switch (tmpdentry->DIR_Name[x])
                {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                    if (tmpdentry->DIR_NTRes & LOWERCASE_EXT)
                    {
                        if (tmpdentry->DIR_Name[x] + 32 == dest_dentry->name[j])
                        {
                            j++;
                            break;
                        }
                        else
                        {
                            goto continue_cmp_fail;
                        }
                    }
                    else
                    {
                        if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                        {
                            j++;
                            break;
                        }
                        else
                        {
                            goto continue_cmp_fail;
                        }
                    }

                case '0' ... '9':
                    if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }

                case ' ':
                    if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                    {
                        goto continue_cmp_fail;
                    }

                default:
                    goto continue_cmp_fail;
                }
            }
        }
        goto find_lookup_success;

    continue_cmp_fail:;
    }

    cluster = readfatentry(fsbi, cluster);
    if (cluster < 0x0ffffff7)
    {
        goto next_cluster;
    }
    kfree(buff);
    return NULL;

find_lookup_success:
    p = (struct index_node *)kmalloc(sizeof(struct index_node), 0);
    memset(p, 0, sizeof(struct index_node));
    p->file_size = tmpdentry->DIR_FileSize;
    p->blocks = (p->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_sector;
    p->attribute = (tmpdentry->DIR_Attr & ATTR_DIRECTORY) ? FS_ATTR_DIR : FS_ATTR_FILE;
    p->sb = parent_inode->sb;
    p->f_ops = &FAT32_file_ops;
    p->inode_ops = &FAT32_inode_ops;

    p->private_index_info = (struct FAT32_inode_info *)kmalloc(sizeof(struct FAT32_inode_info), 0);
    memset(p->private_index_info, 0, sizeof(struct FAT32_inode_info));
    finode = p->private_index_info;

    finode->first_cluster = (tmpdentry->DIR_FstClusHI << 16 | tmpdentry->DIR_FstClusLO) & 0x0fffffff;
    finode->dentry_location = cluster;
    finode->dentry_position = tmpdentry - (struct FAT32_D *)buff;
    finode->create_date = tmpdentry->DIR_CrtTime;
    finode->create_time = tmpdentry->DIR_CrtDate;
    finode->write_date = tmpdentry->DIR_WrtTime;
    finode->write_time = tmpdentry->DIR_WrtDate;
    dest_dentry->dir_inode = p;
    kfree(buff);
    return dest_dentry;
}
struct index_node_operations FAT32_inode_ops =
    {
        .create = FAT32_create,
        .lookup = FAT32_lookup,
        .mkdir = FAT32_mkdir,
        .rmdir = FAT32_rmdir,
        .rename = FAT32_rename,
        .getattr = FAT32_getattr,
        .setattr = FAT32_setattr,
};
long FAT32_compare(struct dir_entry *parent_dentry, char *source_filename, char *destination_filename)
{
    return 1;
}
long FAT32_hash(struct dir_entry *dentry, char *filename)
{
    return 1;
}
long FAT32_release(struct dir_entry *dentry)
{
    return 1;
}
long FAT32_iput(struct dir_entry *dentry, struct index_node *inode)
{
    return 1;
}
struct dir_entry_operations FAT32_dentry_ops =
    {
        .compare = FAT32_compare,
        .hash = FAT32_hash,
        .release = FAT32_release,
        .iput = FAT32_iput,
};
void fat32_write_superblock(struct super_block *sb)
{
    return;
}

void fat32_put_superblock(struct super_block *sb)
{
    kfree(sb->private_sb_info);
    kfree(sb->root->dir_inode->private_index_info);
    kfree(sb->root->dir_inode);
    kfree(sb->root);
    kfree(sb);
    return;
}

void fat32_write_inode(struct index_node *inode)
{
    struct FAT32_D *fdentry = NULL;
    struct FAT32_D *buff = NULL;
    struct FAT32_inode_info *finode = inode->private_index_info;
    struct FAT32_sb_info *fsbi = inode->sb->private_sb_info;
    unsigned long sector = 0;

    if (finode->dentry_location == 0)
    {
        color_printk(RED, BLACK, "FS ERROR:write root inode!\n");
        return;
    }
    sector = fsbi->Data_firstsector + (finode->dentry_location - 2) * fsbi->sector_per_cluster;
    buff = (struct FAT32_D *)kmalloc(fsbi->bytes_per_cluster, 0);
    memset(buff, 0, fsbi->bytes_per_cluster);
    disk_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buff);
    fdentry = buff + finode->dentry_position;

    ////alert fat32 dentry data
    fdentry->DIR_FileSize = inode->file_size;
    fdentry->DIR_FstClusLO = finode->first_cluster & 0xffff;
    fdentry->DIR_FstClusHI = (fdentry->DIR_FstClusHI & 0xf000) | (finode->first_cluster >> 16);

    disk_device_operation.transfer(ATA_WRITE_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buff);
    kfree(buff);
    return;
}

struct super_block_operations FAT32_sb_ops =
    {
        .write_superblock = fat32_write_superblock,
        .put_superblock = fat32_put_superblock,

        .write_inode = fat32_write_inode,
};
struct super_block *fat32_read_superblock(struct disk_entry *entry, void *buff)
{
    struct super_block *sbp = NULL;
    struct FAT32_inode_info *finode = NULL;
    struct FAT32_BootSector *fbs = NULL;
    struct FAT32_sb_info *fsbi = NULL;

    ////super block
    sbp = (struct super_block *)kmalloc(sizeof(struct super_block), 0);
    memset(sbp, 0, sizeof(struct super_block));

    sbp->sb_ops = &FAT32_sb_ops;
    sbp->private_sb_info = (struct FAT32_sb_info *)kmalloc(sizeof(struct FAT32_sb_info), 0);
    memset(sbp->private_sb_info, 0, sizeof(struct FAT32_sb_info));

    ////fat32 boot sector
    fbs = (struct FAT32_BootSector *)buff;
    fsbi = sbp->private_sb_info;
    fsbi->start_sector = entry->startlba;
    fsbi->sector_count = entry->sectornum;
    fsbi->sector_per_cluster = fbs->BPB_SecPerClus;
    fsbi->bytes_per_cluster = fbs->BPB_SecPerClus * fbs->BPB_BytesPerSec;
    fsbi->bytes_per_sector = fbs->BPB_BytesPerSec;
    fsbi->Data_firstsector = entry->startlba + fbs->BPB_RsvdSecCnt + fbs->BPB_FATSz32 * fbs->BPB_NumFATs;
    fsbi->FAT1_firstsector = entry->startlba + fbs->BPB_RsvdSecCnt;
    fsbi->sector_per_FAT = fbs->BPB_FATSz32;
    fsbi->NumFATs = fbs->BPB_NumFATs;
    fsbi->fsinfo_sector_infat = fbs->BPB_FSInfo;
    fsbi->bootsector_bk_infat = fbs->BPB_BkBootSec;

    color_printk(BLUE, BLACK, "FAT32 Boot Sector\tBPB_FSInfo:%#018lx\tBPB_BkBootSec:%#018lx\tBPB_TotSec32:%#018lx\n", fbs->BPB_FSInfo, fbs->BPB_BkBootSec, fbs->BPB_TotSec32);

    ////fat32 fsinfo sector
    fsbi->fat_fsinfo = (struct FAT32_FSInfo *)kmalloc(sizeof(struct FAT32_FSInfo), 0);
    memset(fsbi->fat_fsinfo, 0, 512);
    disk_device_operation.transfer(ATA_READ_CMD, entry->startlba + fbs->BPB_FSInfo, 1, (unsigned char *)fsbi->fat_fsinfo);

    color_printk(BLUE, BLACK, "FAT32 FSInfo\tFSI_LeadSig:%#018lx\tFSI_StrucSig:%#018lx\tFSI_Free_Count:%#018lx\n", fsbi->fat_fsinfo->FSI_LeadSig, fsbi->fat_fsinfo->FSI_StrucSig, fsbi->fat_fsinfo->FSI_Free_Count);

    ////directory entry
    sbp->root = (struct dir_entry *)kmalloc(sizeof(struct dir_entry), 0);
    memset(sbp->root, 0, sizeof(struct dir_entry));

    list_init(&sbp->root->child_node);
    list_init(&sbp->root->subdirs_list);
    sbp->root->parent = sbp->root;
    sbp->root->dir_ops = &FAT32_dentry_ops;
    sbp->root->name = (char *)kmalloc(2, 0);
    sbp->root->name[0] = '/';
    sbp->root->name_length = 1;

    ////index node
    sbp->root->dir_inode = (struct index_node *)kmalloc(sizeof(struct index_node), 0);
    memset(sbp->root->dir_inode, 0, sizeof(struct index_node));
    sbp->root->dir_inode->inode_ops = &FAT32_inode_ops;
    sbp->root->dir_inode->f_ops = &FAT32_file_ops;
    sbp->root->dir_inode->file_size = 0;
    sbp->root->dir_inode->blocks = (sbp->root->dir_inode->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_sector;
    sbp->root->dir_inode->attribute = FS_ATTR_DIR;
    sbp->root->dir_inode->sb = sbp;

    ////fat32 root inode
    sbp->root->dir_inode->private_index_info = (struct FAT32_inode_info *)kmalloc(sizeof(struct FAT32_inode_info), 0);
    memset(sbp->root->dir_inode->private_index_info, 0, sizeof(struct FAT32_inode_info));
    finode = (struct FAT32_inode_info *)sbp->root->dir_inode->private_index_info;
    finode->first_cluster = fbs->BPB_RootClus;
    finode->dentry_location = 0;
    finode->dentry_position = 0;
    finode->create_date = 0;
    finode->create_time = 0;
    finode->write_date = 0;
    finode->write_time = 0;

    return sbp;
}
struct file_system_type FAT32_fs_type =
    {
        .name = "FAT32",
        .fs_flags = 0,
        .read_superblock = fat32_read_superblock,
        .next = NULL,
};
#include "sys.c"
void disk_fat32_fs_init()
{
    int i = 0;
    unsigned char buffer[512];
    memset(buffer, 0, 512);
    disk_device_operation.transfer(ATA_READ_CMD, FAT32_START_SECTOR, 1, buffer);
    struct disk_entry entry;
    entry.startlba = FAT32_START_SECTOR;
    entry.sectornum = ((struct FAT32_BootSector *)buffer)->BPB_TotSec32;
    register_filesystem(&FAT32_fs_type);
    root_sb = mount_fs("FAT32", &entry, buffer);
    return;
}
