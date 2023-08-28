#include "memory.h"
#include "UEFI.h"
#include "task.h"
#include "errno.h"
struct SLAB_cache kmalloc_cache_size[16] = 
{
    {32, 0, 0, NULL, NULL, NULL, NULL},
    {64, 0, 0, NULL, NULL, NULL, NULL},
    {128, 0, 0, NULL, NULL, NULL, NULL},
    {256, 0, 0, NULL, NULL, NULL, NULL},
    {512, 0, 0, NULL, NULL, NULL, NULL},
    {1024, 0, 0, NULL, NULL, NULL, NULL},
    {2048, 0, 0, NULL, NULL, NULL, NULL},
    {4096, 0, 0, NULL, NULL, NULL, NULL},
    {8192, 0, 0, NULL, NULL, NULL, NULL},
    {16384, 0, 0, NULL, NULL, NULL, NULL},
    {32768, 0, 0, NULL, NULL, NULL, NULL},
    {65536, 0, 0, NULL, NULL, NULL, NULL},
    {131072, 0, 0, NULL, NULL, NULL, NULL},
    {262144, 0, 0, NULL, NULL, NULL, NULL},
    {524288, 0, 0, NULL, NULL, NULL, NULL},
    {1048576, 0, 0, NULL, NULL, NULL, NULL}
};
int ZONE_DMA_INDEX = 0;
int ZONE_NORMAL_INDEX = 0;
int ZONE_UNMAPED_INDEX = 0;
unsigned long* GLOBAL_CR3 = NULL;
extern struct KERNEL_BOOT_PARAMETER_INFORMATION *boot_para_info;

unsigned long page_init(struct Page *page, unsigned long flags)
{
    page->attribute |= flags;
    if(!page->reference_count || (page->attribute & PAGE_SHARED))
    {
        page->reference_count++;
        page->zone_struct->total_pages_link++;
    }
    return 1;
}
void init_memory(void)
{
    int i, j;
    unsigned long total_mem = 0;
    struct EFI_E820_MEMORY_DESCRIPTOR *p = NULL;
    color_printk(BLUE, BLACK, "Display Physical Address Map, Type(1:RAM, 2:ROM or Reserved, 3:ACPI Reclaim Memory, 4:ACPI NVS Memory, Others:Undefined)\n");
    p = (struct EFI_E820_MEMORY_DESCRIPTOR *)boot_para_info->E820_Info.E820_Entry;
    unsigned long m_max = 0, m_tmp = 0;
    for (i = 0; i < 32; i++)
    {
        if (p->type == 1)
        {
            total_mem = total_mem + p->length;
        }
        else if (p->type > 4 || p->length == 0 || p->type < 1)
        {
            break;
        }
        m_tmp = p->address + p->length;
        if(m_tmp > m_max)
        {
            m_max = m_tmp;
        }
        mem_structure.E820[i].address += p->address;
        mem_structure.E820[i].length += p->length;
        mem_structure.E820[i].type = p->type;
        mem_structure.E820_length = i;
        color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#018lx\n", p->address, p->length, p->type);
        p++;
    }
    color_printk(ORANGE, BLACK, "OS Can Used Totol RAM = %#018x\n", total_mem);
    int All_Page = 0;
    for (i = 0; i < mem_structure.E820_length; i++)
    {
        unsigned long start, end;
        if (mem_structure.E820[i].type != 1)
        {
            continue;
        }
        start = PAGE_2M_ALIGN(mem_structure.E820[i].address);
        end = (mem_structure.E820[i].address + mem_structure.E820[i].length) & PAGE_2M_MASK;
        if (end <= start)
        {
            continue;
        }
        All_Page = All_Page + ((end - start) >> PAGE_2M_SHIFT);
        color_printk(ORANGE, BLACK, "Total effective 2MB Pages:%#010x = %#010d\n", All_Page, All_Page);
    }
    total_mem = m_max;
    mem_structure.bits_map = (unsigned long *)PAGE_4K_ALIGN(mem_structure.start_brk);
    mem_structure.bits_size = total_mem >> PAGE_2M_SHIFT;
    mem_structure.bits_length = ((mem_structure.bits_size + sizeof(unsigned long) - 1) / 8) & (~(sizeof(unsigned long) - 1));
    memset(mem_structure.bits_map, 0xff, mem_structure.bits_length);
    mem_structure.pages_struct = (struct Page *)PAGE_4K_ALIGN((unsigned long)mem_structure.bits_map + mem_structure.bits_length);
    mem_structure.pages_size = mem_structure.bits_size;
    mem_structure.pages_length = (mem_structure.pages_size * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    memset(mem_structure.pages_struct, 0x00, mem_structure.pages_length);
    mem_structure.zones_struct = (struct Zone *)PAGE_4K_ALIGN((unsigned long)mem_structure.pages_struct + mem_structure.pages_length);
    mem_structure.zones_size = 0;
    mem_structure.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    memset(mem_structure.zones_struct, 0x00, mem_structure.zones_length);
    for (i = 0; i <= mem_structure.E820_length; i++)
    {
        unsigned long start, end;
        struct Page *p;
        struct Zone *z;
        if (mem_structure.E820[i].type != 1)
        {
            continue;
        }
        start = PAGE_2M_ALIGN(mem_structure.E820[i].address);
        end = ((mem_structure.E820[i].address + mem_structure.E820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if (end <= start)
        {
            continue;
        }
        z = mem_structure.zones_struct + mem_structure.zones_size;
        mem_structure.zones_size++;
        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = end - start;
        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;
        z->total_pages_link = 0;
        z->attritube = 0;
        z->GMD_struct = &mem_structure;
        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page *)(mem_structure.pages_struct + (start >> PAGE_2M_SHIFT));
        p = z->pages_group;
        for (j = 0; j < z->pages_length; j++, p++)
        {
            p->zone_struct = z;
            p->p_address = start + PAGE_2M_SIZE * j;
            p->attribute = 0;
            p->age = 0;
            p->reference_count = 0;
            *(mem_structure.bits_map + ((p->p_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->p_address >> PAGE_2M_SHIFT) % 64;
        }
    }
    mem_structure.E820[0].type != 1;
    mem_structure.pages_struct->zone_struct = mem_structure.zones_struct;
    mem_structure.pages_struct->p_address = (unsigned long)0;
    set_page_attribute(mem_structure.pages_struct, PAGE_PT_MAPED | PAGE_KERNEL_INIT | PAGE_KERNEL);
    mem_structure.pages_struct->reference_count = 1;
    mem_structure.pages_struct->age = 0;
    mem_structure.zones_length = (mem_structure.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    color_printk(ORANGE, BLACK, "bits_map:%#018lx bits_size:%#018lx bits_length:%#018lx\n", *mem_structure.bits_map, mem_structure.bits_size, mem_structure.bits_length);
    color_printk(ORANGE, BLACK, "pages_struct:%#018lx pages_size:%#018lx pages_length:%#018lx\n", mem_structure.pages_struct, mem_structure.pages_size, mem_structure.pages_length);
    color_printk(ORANGE, BLACK, "zones_struct:%#018lx zones_size:%#018lx zones_length:%#018lx\n", mem_structure.zones_struct, mem_structure.zones_size, mem_structure.zones_length);
    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;
    ZONE_UNMAPED_INDEX = 0;
    for (i = 0; i < mem_structure.zones_size; i++)
    {
        struct Zone *z = mem_structure.zones_struct + i;
        color_printk(ORANGE, BLACK, "zone_start_address:%#018lx zone_end_address:%#018lx zone_length:%#018lx pages_group:%#018lx pages_length:%#018lx\n", z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);
        if (z->zone_start_address >= 0x100000000 && !ZONE_UNMAPED_INDEX)
        {
            ZONE_UNMAPED_INDEX = i;
        }
    }
	color_printk(ORANGE, BLACK, "ZONE_DMA_INDEX:%d\tZONE_NORMAL_INDEX:%d\tZONE_UNMAPED_INDEX:%d\n", ZONE_DMA_INDEX, ZONE_NORMAL_INDEX, ZONE_UNMAPED_INDEX);
    mem_structure.end_of_struct = (unsigned long)((unsigned long)mem_structure.zones_struct + mem_structure.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));
    color_printk(ORANGE, BLACK, "start_code:%#018lx end_code:%#018lx start_data:%#018lx end_data:%#018lx start_brk:%#018lx end_of_struct:%#018lx\n", mem_structure.start_code, mem_structure.end_code, mem_structure.start_data, mem_structure.end_data, mem_structure.start_brk, mem_structure.end_of_struct);
    i = V_TO_P(mem_structure.end_of_struct) >> PAGE_2M_SHIFT;
    for (j = 0; j <= i; j++)
    {
        struct Page* tmp_page = mem_structure.pages_struct + j;
        page_init(tmp_page, PAGE_PT_MAPED | PAGE_KERNEL_INIT | PAGE_KERNEL);
        *(mem_structure.bits_map + ((tmp_page->p_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (tmp_page->p_address >> PAGE_2M_SHIFT) % 64;
        tmp_page->zone_struct->page_using_count++;
        tmp_page->zone_struct->page_free_count--;
    }
    GLOBAL_CR3 = get_gdt();
    color_printk(INDIGO, BLACK, "GLOBAL_CR3:%#018lx\n", GLOBAL_CR3);
    color_printk(INDIGO, BLACK, "*GLOBAL_CR3:%#018lx\n", *(unsigned long *)P_TO_V(GLOBAL_CR3) & (~0xff));
    color_printk(INDIGO, BLACK, "**GLOBAL_CR3:%#018lx\n", *(unsigned long *)P_TO_V(*(unsigned long *)P_TO_V(GLOBAL_CR3) & (~0xff)) & (~0xff));
    color_printk(ORANGE, BLACK, "1.bits_map:%#018lx\tzone_struct->page_using:%d\tzone_struct->page_free:%d\n", *mem_structure.bits_map, mem_structure.zones_struct->page_using_count, mem_structure.zones_struct->page_free_count);
    // for(i = 0; i < 10; i++)
    // {
    //     *((unsigned long *)(P_TO_V(GLOBAL_CR3) + i)) = (unsigned long)0;
    // }
    flush_tlb();
    return;
}
unsigned long *get_gdt(void)
{
    unsigned long *tmp;
    asm volatile(
        "movq %%cr3, %0     \n"
        : "=r"(tmp)
        :
        : "memory");
    return tmp;
}
struct Page *alloc_pages(int zones_select, int number, unsigned long flags)
{
    int i;
    unsigned long page = 0;
    unsigned long attribute = 0;

    int zone_start = 0;
    int zone_end = 0;
    if(number >= 64 || number <= 0)
    {
        color_printk(RED, BLACK, "alloc_pages() ERROR:number is invalid\n");
        return NULL;
    }
    switch (zones_select)
    {
    case ZONE_DMA:
        zone_start = 0;
        zone_end = ZONE_DMA_INDEX;
        attribute = PAGE_PT_MAPED;
        break;
    case ZONE_NORMAL:
        zone_start = ZONE_DMA_INDEX;
        zone_end = ZONE_NORMAL_INDEX;
        attribute = PAGE_PT_MAPED;
        break;
    case ZONE_UNMAPED:
        zone_start = ZONE_UNMAPED_INDEX;
        zone_end = (mem_structure.zones_size) - 1;
        attribute = 0;
        break;
    default:
        color_printk(RED, BLACK, "Alloc_Pages error zone_select index\n");
        return NULL;
        break;
    }
    for (i = zone_start; i <= zone_end; i++)
    {
        struct Zone *z;
        unsigned long j;
        unsigned long start, end;
        unsigned long tmp;
        if ((mem_structure.zones_struct + i)->page_free_count < number)
        {
            continue;
        }
        z = mem_structure.zones_struct + i;
        start = z->zone_start_address >> PAGE_2M_SHIFT;
        end = z->zone_end_address >> PAGE_2M_SHIFT;
        tmp = 64 - number % 64;
        for (j = start; j <= end; j += j % 64 ? tmp : 64)
        {
            unsigned long *p = mem_structure.bits_map + (j >> 6);
            unsigned long shift = j % 64;
            unsigned long k = 0;
            unsigned long num = (1UL << number) - 1;
            for(k = shift; k < 64; k++)
            {
                if(!((k ? ((*p >> k) | (*(p + 1) << (64 - k))) : *p) & (num)))
                {
                    unsigned long l;
                    page = j + k - shift;
                    for(l = 0; l < number; l++)
                    {
                        struct Page* page_ptr = mem_structure.pages_struct + page + l;
                        *(mem_structure.bits_map + ((page_ptr->p_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page_ptr->p_address >> PAGE_2M_SHIFT) % 64;
                        z->page_using_count++;
                        z->page_free_count--;
                        page_ptr->attribute = attribute;
                    }
                    goto Found;
                }
            }
        }
    }
    color_printk(RED,BLACK,"alloc_pages() ERROR: no page can alloc\n");
    return NULL;
    Found:
        return (struct Page *)(mem_structure.pages_struct + page);
}

struct SLAB* kmalloc_create(unsigned long size)
{
    int i = 0;
    struct SLAB* slab = NULL;
    struct Page* page = NULL;
    unsigned long* Vaddress = NULL;
    long structsize = 0;
    page = alloc_pages(ZONE_NORMAL, 1, 0);
    if(page == NULL)
    {
        color_printk(RED, BLACK, "kmalloc_create()->page_alloc() ERROR page = NULL\n");
        return NULL;
    }
    page_init(page, PAGE_KERNEL);
    switch(size)
    {
        case 32:
        case 64:
        case 128:
        case 256:
        case 512:
            Vaddress = (unsigned long*)P_TO_V(page->p_address);
            structsize = sizeof(struct SLAB) + PAGE_2M_SIZE / size / 8;
            slab = (struct SLAB*)((unsigned char*)Vaddress + PAGE_2M_SIZE - structsize);
            slab->color_map = (unsigned long*)((unsigned char*)slab + sizeof(struct SLAB));
            slab->free_count = (PAGE_2M_SIZE - (PAGE_2M_SIZE / size / 8) - sizeof(struct SLAB)) / size;
            slab->using_count = 0;
            slab->color_count = slab->free_count;
            slab->Vaddress = Vaddress;
            slab->page = page;
            list_init(&slab->list);
            slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
            memset(slab->color_map, 0xff, slab->color_length);
            for(i = 0; i < slab->color_count; i++)
            {
                *(slab->color_map + (i >> 6)) ^= 1UL << i % 64;
            }
            break;
        case 1024: //1KB
        case 2048:
        case 4096: //4KB
        case 8192:
        case 16384:
        case 32768:
        case 65536:
        case 131072:
        case 262144:
        case 524288:
        case 1048576:
            slab = (struct SLAB*)kmalloc(sizeof(struct SLAB), 0);
            slab->free_count = PAGE_2M_SIZE / size;
            slab->using_count = 0;
            slab->color_count = slab->free_count;
            slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
            slab->color_map = (unsigned long*)kmalloc(slab->color_length, 0);
            memset(slab->color_map, 0xff, slab->color_length);
            slab->Vaddress = (unsigned long*)P_TO_V(page->p_address);
            slab->page = page;
            list_init(&slab->list);
            for(i = 0; i < slab->color_count; i++)
            {
                *(slab->color_map + (i >> 6)) ^= 1UL << i % 64;
            }
            break;
        default:
            color_printk(RED, BLACK, "kmalloc_create() ERROR:wrong size%08d\n", size);
            free_pages(page, 1);
            return NULL;
    }
    return slab;
}

void* kmalloc(unsigned long size, unsigned long flags)
{
    int i = 0;
    int j = 0;
    struct SLAB* slab = NULL;
    //1048576 = 1MB
    if(size > 1048576)
    {
        color_printk(RED, BLACK, "kmalloc() ERROR:kmalloc size too long:%08d\n", size);
        return NULL;
    }
    for(i = 0; i < 16; i++)
    {
        if(kmalloc_cache_size[i].size >= size)
        {
            break;
        }
    }
    slab = kmalloc_cache_size[i].cache_pool;
    if(kmalloc_cache_size[i].total_free != 0)
    {
        do
        {
            if(slab->free_count == 0)
            {
                slab = container_of(list_next(&slab->list), struct SLAB, list);
            }
            else
            {
                break;
            }
        }while(slab != kmalloc_cache_size[i].cache_pool);
    }
    else
    {
        slab = kmalloc_create(kmalloc_cache_size[i].size);
        if(slab == NULL)
        {
            color_printk(RED, BLACK, "kmalloc()->kmalloc_create()->slab == NULL\n");
            return NULL;
        }
        kmalloc_cache_size[i].total_free += slab->color_count;
        color_printk(BLUE, BLACK, "kmalloc()->kmalloc_create() <= size:%#010x\n", kmalloc_cache_size[i].size);
        list_add_before(&kmalloc_cache_size[i].cache_pool->list, &slab->list);
    }
    for(j = 0; j < slab->color_count; j++)
    {
        if(*(slab->color_map + (j >> 6)) == 0xffffffffffffffffUL)
        {
            j += 63;
            continue;
        }
        if((*(slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0)
        {
            *(slab->color_map + (j >> 6)) |= 1UL << (j % 64);
            slab->using_count++;
            slab->free_count--;
            kmalloc_cache_size[i].total_using++;
            kmalloc_cache_size[i].total_free--;
            return (void *)((char*)slab->Vaddress + kmalloc_cache_size[i].size * j);
        }
    }
    color_printk(RED, BLACK, "kmalloc() ERROR: no memory call alloc\n");
    return NULL;
}
struct SLAB_cache* slab_create(unsigned long size, void* (*construct)(void* Vaddress, unsigned long arg), void* (*destruct)(void* Vaddress, unsigned long arg), unsigned long arg)
{
    struct SLAB_cache* slab_cache = NULL;
    slab_cache = (struct SLAB_cache*)kmalloc(sizeof(struct SLAB_cache), 0);
    if(slab_cache == NULL)
    {
        color_printk(RED, BLACK, "slab_create()->slab_cache = NULL\n");
        return NULL;
    }
    memset(slab_cache, 0, sizeof(struct SLAB_cache));
    slab_cache->size = SIZEOF_LONG_ALIGN(size);
    slab_cache->total_using = 0;
    slab_cache->total_free = 0;
    slab_cache->cache_pool = (struct SLAB*)kmalloc(sizeof(struct SLAB), 0);
    if(slab_cache->cache_pool == NULL)
    {
        color_printk(RED, BLACK, "slab_cache()->kmalloc(slab_cache->cache_pool) ERROR == NULL\n");
        kfree(slab_cache);
        return NULL;
    }
    memset(slab_cache->cache_pool, 0, sizeof(struct SLAB));
    slab_cache->dma_cache_pool = NULL;
    slab_cache->construct = construct;
    slab_cache->destruct = destruct;
    list_init(&slab_cache->cache_pool->list);
    slab_cache->cache_pool->page = alloc_pages(ZONE_NORMAL, 1, 0);
    if(slab_cache->cache_pool->page == NULL)
    {
        color_printk(RED, BLACK, "slab_cache()->alloc_pages() page == NULL\n");
        kfree(slab_cache->cache_pool);
        kfree(slab_cache);
        return NULL;
    }
    page_init(slab_cache->cache_pool->page, PAGE_KERNEL);
    slab_cache->cache_pool->using_count = 0;
    slab_cache->cache_pool->free_count = PAGE_2M_SIZE / slab_cache->size;
    slab_cache->total_free = slab_cache->cache_pool->free_count;
    slab_cache->cache_pool->Vaddress = (unsigned long*)P_TO_V(slab_cache->cache_pool->page->p_address);
    slab_cache->cache_pool->color_count = slab_cache->cache_pool->free_count;
    slab_cache->cache_pool->color_length = ((slab_cache->cache_pool->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
    slab_cache->cache_pool->color_map = (unsigned long*)kmalloc(slab_cache->cache_pool->color_length, 0);
    if(slab_cache->cache_pool->color_map == NULL)
    {
        color_printk(RED, BLACK, "slab_create()->color_map == NULL\n");
        free_pages(slab_cache->cache_pool->page, 1);
        kfree(slab_cache->cache_pool);
        kfree(slab_cache);
        return NULL;
    }
    memset(slab_cache->cache_pool->color_map, 0, slab_cache->cache_pool->color_length);
    return slab_cache;
}
unsigned long slab_destroy(struct SLAB_cache* slab_cache)
{
    struct SLAB* slab_pool = slab_cache->cache_pool;
    struct SLAB* slab_tmp = NULL;
    if(slab_cache->total_using != 0)
    {
        color_printk(RED, BLACK, "slab_cache->total_using != 0\n");
        return 0;
    }
    while(!list_is_empty(&slab_pool->list))
    {
        slab_tmp = slab_pool;
        slab_pool = (struct SLAB*)container_of(list_next(&slab_pool->list), struct SLAB, list);
        list_delete(&slab_tmp->list);
        kfree(slab_tmp->color_map);
        page_clean(slab_tmp->page);
        free_pages(slab_tmp->page, 1);
        kfree(slab_tmp);
    }
    kfree(slab_pool->color_map);
    page_clean(slab_pool->page);
    free_pages(slab_pool->page, 1);
    kfree(slab_pool);
    return 1;
}
void* slab_malloc(struct SLAB_cache* slab_cache, unsigned long arg)
{
    struct SLAB* slab_pool = slab_cache->cache_pool;
    struct SLAB* slab_tmp = NULL;
    int j = 0;
    if(slab_cache->total_free == 0)
    {
        slab_tmp = (struct SLAB*)kmalloc(sizeof(struct SLAB), 0);
        if(slab_tmp == NULL)
        {
            color_printk(RED, BLACK, "slab_malloc()->kmalloc slab_tmp == NULL\n");
            return NULL;
        }
        memset(slab_tmp, 0, sizeof(struct SLAB));
        list_init(&slab_tmp->list);
        slab_tmp->page = alloc_pages(ZONE_NORMAL, 1, 0);
        if(slab_tmp->page == NULL)
        {
            color_printk(RED, BLACK, "slab_malloc()->alloc_page == NULL\n");
            kfree(slab_tmp);
            return NULL;
        }
        page_init(slab_tmp->page, PAGE_KERNEL);
        slab_tmp->using_count = 0;
        slab_tmp->free_count = PAGE_2M_SIZE / slab_cache->size;
        slab_tmp->Vaddress = (unsigned long*)P_TO_V(slab_tmp->page->p_address);
        slab_tmp->color_count = slab_tmp->free_count;
        slab_tmp->color_length = ((slab_tmp->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        slab_tmp->color_map = (unsigned long*)kmalloc(slab_tmp->color_length, 0);
        if(slab_tmp->color_map == NULL)
        {
            color_printk(RED, BLACK, "slab_malloc()->kmalloc(colormap) == NULL\n");
            free_pages(slab_tmp->page, 1);
            kfree(slab_tmp);
            return NULL;
        }
        memset(slab_tmp->color_map, 0, slab_tmp->color_length);
        list_add_behind(&slab_cache->cache_pool->list, &slab_tmp->list);
        slab_cache->total_free += slab_tmp->color_count;
        for(j = 0; j < slab_tmp->color_count; j++)
        {
            if((*(slab_tmp->color_map + (j >> 6)) & 1UL << (j % 64)) == 0)
            {
                *(slab_tmp->color_map + (j >> 6)) |= 1UL << (j % 64);
                slab_tmp->using_count++;
                slab_tmp->free_count--;
                slab_cache->total_free--;
                slab_cache->total_using++;
                if(slab_cache->construct != NULL)
                {
                    return slab_cache->construct((char*)slab_tmp->Vaddress + slab_cache->size * j, arg);
                }
                else
                {
                    return (void*)((char*)slab_tmp->Vaddress + slab_cache->size * j);
                }
            }
        }
    }
    else
    {
        do
        {
            if(slab_pool->free_count == 0)
            {
                slab_pool = (struct SLAB*)container_of(list_next(&slab_pool->list), struct SLAB, list);
                continue;
            }
            for(j = 0; j < slab_pool->color_count; j++)
            {
                if(*(slab_pool->color_map + (j >> 6)) == 0xffffffffffffffffUL)
                {
                    j += 63;
                    continue;
                }
            }
            if((*(slab_pool->color_map + (j >> 6)) & (1UL << (j % 64))) == 0)
            {
                *(slab_pool->color_map + (j >> 6)) |= 1UL << (j % 64);
                slab_pool->using_count++;
                slab_pool->free_count--;
                if(slab_cache->construct != NULL)
                {
                    return slab_cache->construct((char*)slab_tmp->Vaddress + slab_cache->size * j, arg);
                }
                else
                {
                    return (void*)((char*)slab_tmp->Vaddress + slab_cache->size * j);
                }
            }
        }while(slab_pool != slab_cache->cache_pool);
    }
    color_printk(RED, BLACK, "slab_malloc()ERROR: can't alloc\n");
    if(slab_tmp != NULL)
    {
        list_delete(&slab_tmp->list);
        kfree(slab_tmp->color_map);
        page_clean(slab_tmp->page);
        free_pages(slab_tmp->page, 1);
        kfree(slab_tmp);
    }
    return NULL;
}
unsigned long slab_free(struct SLAB_cache* slab_cache, void* address, unsigned long arg)
{
    struct SLAB* slab_pool = slab_cache->cache_pool;
    int index = 0;
    do
    {
        if(slab_pool->Vaddress <= address && address < slab_pool->Vaddress + PAGE_2M_SIZE)
        {
            index = (address - slab_pool->Vaddress) / slab_cache->size;
            *(slab_pool->color_map + (index >> 6)) ^= 1UL << (index % 64);
            slab_pool->using_count--;
            slab_pool->free_count++;
            slab_cache->total_free++;
            slab_cache->total_using--;
            if(slab_cache->destruct != NULL)
            {
                slab_cache->destruct((char*)slab_pool->Vaddress + slab_cache->size * index, arg);
            }
            if((slab_pool->using_count == 0) && (slab_cache->total_free >= slab_pool->color_count * 3 / 2))
            {
                list_delete(&slab_pool->list);
                slab_cache->total_free -= slab_pool->color_count;
                kfree(slab_pool->color_map);
                page_clean(slab_pool->page);
                free_pages(slab_pool->page, 1);
                kfree(slab_pool);
            }
            return 1;
        }
        else
        {
            slab_pool = (struct SLAB*)container_of(list_next(&slab_pool->list), struct SLAB, list);
            continue;
        }
    }while(slab_pool != slab_cache->cache_pool);
    color_printk(RED, BLACK, "slab_free() ERROR address is not in slab\n");
    return 0;
}
unsigned long page_clean(struct Page * page)
{
    page->reference_count--;
    page->zone_struct->total_pages_link--;
    if(!page->reference_count)
    {
        page->attribute &= PAGE_PT_MAPED;
    }
    return 1;
}
void free_pages(struct Page* page, int number)
{
    int i = 0;
    if(page == NULL)
    {
        color_printk(RED, BLACK, "free_pages()ERROR:page is invalid\n");
        return;
    }
    if(number >= 64 || number <= 0)
    {
        color_printk(RED, BLACK, "free_pages()ERROR:number is invalid\n");
        return;
    }
    for(i = 0; i < number; i++, page++)
    {
        *(mem_structure.bits_map +((page->p_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL << (page->p_address >> PAGE_2M_SHIFT) % 64);
        page->zone_struct->page_using_count--;
        page->zone_struct->page_free_count++;
        page->attribute = 0;
    }
    return;
}
unsigned long kfree(void* address)
{
    int i = 0;
    int index = 0;
    struct SLAB* slab = NULL;
    void* page_bass_address = (void*)((unsigned long)address & PAGE_2M_MASK);
    for(i = 0; i < 16; i++)
    {
        slab = kmalloc_cache_size[i].cache_pool;
        do
        {
            if(slab->Vaddress == page_bass_address)
            {
                index = (address - slab->Vaddress) / kmalloc_cache_size[i].size;
                *(slab->color_map + (index >> 6)) ^= 1UL << index % 64;
                slab->free_count++;
                slab->using_count--;
                kmalloc_cache_size[i].total_free++;
                kmalloc_cache_size[i].total_using--;
                if((slab->using_count == 0) && (kmalloc_cache_size[i].total_free >= slab->color_count * 3 / 2) && kmalloc_cache_size[i].cache_pool != slab)
                {
                    switch(kmalloc_cache_size[i].size)
                    {
                        case 32:
                        case 64:
                        case 128:
                        case 256:
                        case 512:
                            list_delete(&slab->list);
                            kmalloc_cache_size[i].total_free -= slab->color_count;
                            page_clean(slab->page);
                            free_pages(slab->page, 1);
                            break;
                        default:
                            list_delete(&slab->list);
                            kmalloc_cache_size[i].total_free -= slab->color_count;
                            kfree(slab->color_map);
                            page_clean(slab->page);
                            free_pages(slab->page, 1);
                            kfree(slab);
                            break;
                    }
                }
                return 1;
            }
            else
            {
                slab = (struct SLAB*)container_of(list_next(&slab->list), struct SLAB, list);
            }
        }while(slab != kmalloc_cache_size[i].cache_pool);
    }
    color_printk(RED, BLACK, "kfree() ERROR:can't free memory\n");
    return 0;
}
unsigned long slab_init(void)
{
    struct Page* page = NULL;
    unsigned long* virtual = NULL;
    unsigned long i;
    unsigned long j;
    unsigned long tmp_address = mem_structure.end_of_struct;
    for(i = 0; i < 16; i++)
    {
        kmalloc_cache_size[i].cache_pool = (struct SLAB*)mem_structure.end_of_struct;
        mem_structure.end_of_struct = mem_structure.end_of_struct + sizeof(struct SLAB) + sizeof(long) * 10;
        list_init(&kmalloc_cache_size[i].cache_pool->list);
        kmalloc_cache_size[i].cache_pool->using_count = 0;
        kmalloc_cache_size[i].cache_pool->free_count = PAGE_2M_SIZE / kmalloc_cache_size[i].size;
        kmalloc_cache_size[i].cache_pool->color_length = ((PAGE_2M_SIZE / kmalloc_cache_size[i].size + sizeof(unsigned long) * 8 - 1) >> 6) <<3;
        kmalloc_cache_size[i].cache_pool->color_count = kmalloc_cache_size[i].cache_pool->free_count;
        kmalloc_cache_size[i].cache_pool->color_map = (unsigned long*)mem_structure.end_of_struct;
        mem_structure.end_of_struct = (unsigned long)(mem_structure.end_of_struct + kmalloc_cache_size[i].cache_pool->color_length + sizeof(unsigned long) * 10) & (~(sizeof(unsigned long) - 1));
        memset(kmalloc_cache_size[i].cache_pool->color_map, 0xff, kmalloc_cache_size[i].cache_pool->color_length);
        for(j = 0; j < kmalloc_cache_size[i].cache_pool->color_count; j++)
        {
            *(kmalloc_cache_size[i].cache_pool->color_map + (j >> 6)) ^= 1UL << (j % 64);
        }
        kmalloc_cache_size[i].total_free = kmalloc_cache_size[i].cache_pool->color_count;
        kmalloc_cache_size[i].total_using = 0;
    }
    i = V_TO_P(mem_structure.end_of_struct) >> PAGE_2M_SHIFT;
    for(j = PAGE_2M_ALIGN(V_TO_P(tmp_address)) >> PAGE_2M_SHIFT; j <= i; j++)
    {
        page = mem_structure.pages_struct + j;
        *(mem_structure.bits_map + ((page->p_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->p_address >> PAGE_2M_SHIFT) % 64;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page, PAGE_PT_MAPED | PAGE_KERNEL_INIT | PAGE_KERNEL);
    }
    color_printk(ORANGE, BLACK, "2.bits_map:%#018lx\tzone_struct->page_using:%d\tzone_struct->page_free:%d\n", *mem_structure.bits_map, mem_structure.zones_struct->page_using_count, mem_structure.zones_struct->page_free_count);
    for(i = 0; i < 16; i++)
    {
        virtual = (unsigned long*)((mem_structure.end_of_struct + PAGE_2M_SIZE * i + PAGE_2M_SIZE - 1) & PAGE_2M_MASK);
        page = (struct Page*)V_TO_2M(virtual);
        *(mem_structure.bits_map + ((page->p_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->p_address >> PAGE_2M_SHIFT) % 64;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page, PAGE_PT_MAPED | PAGE_KERNEL_INIT | PAGE_KERNEL);
        kmalloc_cache_size[i].cache_pool->page = page;
        kmalloc_cache_size[i].cache_pool->Vaddress = virtual;
    }
    color_printk(ORANGE, BLACK, "3.bits_map:%#018lx\tzone_struct->page_using:%d\tzone_struct->page_free:%d\n", *mem_structure.bits_map, mem_structure.zones_struct->page_using_count, mem_structure.zones_struct->page_free_count);
    color_printk(ORANGE, BLACK, "start_code:%#018lx end_code:%#018lx start_data:%#018lx end_data:%#018lx start_brk:%#018lx end_of_struct:%#018lx\n", mem_structure.start_code, mem_structure.end_code, mem_structure.start_data, mem_structure.end_data, mem_structure.start_brk, mem_structure.end_of_struct);
    return 1;
}
unsigned long get_page_attribute(struct Page* page)
{
    if(page == NULL)
    {
        color_printk(RED, BLACK, "get_page_attribute() ERROR:page == NULL\n");
        return 0;
    }
    return (page->attribute);
}
unsigned long set_page_attribute(struct Page* page, unsigned long flags)
{
    if(page == NULL)
    {
        color_printk(RED, BLACK, "set_page_attribute() ERROR:page == NULL\n");
        return 0;
    }
    page->attribute = flags;
    return 1;
}
void pagetable_init()
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long* tmp = NULL;
    unsigned long* virtual;
    GLOBAL_CR3 = get_gdt();
    tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)GLOBAL_CR3 & (~0xfffUL))) + 8 * 256);
    color_printk(YELLOW, BLACK, "1:%#018lx\t%#018lx\n", (unsigned long)tmp, *tmp);
    tmp = (unsigned long*)P_TO_V(*tmp & (~0xfffUL));
    color_printk(YELLOW, BLACK, "2:%#018lx\t%#018lx\n", (unsigned long)tmp, *tmp);
    tmp = (unsigned long*)P_TO_V(*tmp & (~0xfffUL));
    color_printk(YELLOW, BLACK, "3:%#018lx\t%#018lx\n", (unsigned long)tmp, *tmp);
    for(i = 0; i < mem_structure.zones_size; i++)
    {
        struct Zone* z = mem_structure.zones_struct + i;
        struct Page* page = z->pages_group;
        if(ZONE_UNMAPED_INDEX && i == ZONE_UNMAPED_INDEX)
        {
            break;
        }
        for(j = 0; j < z->pages_length; j++, page++)
        {
            tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)GLOBAL_CR3 & (~0xfffUL))) + (((unsigned long)P_TO_V(page->p_address) >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
            if(*tmp == 0)
            {
                virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
                memset(virtual, 0, PAGE_4K_SIZE);
                set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_KERNEL_GDT));
            }
            tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + (((unsigned long)P_TO_V(page->p_address) >> PAGE_1G_SHIFT) & 0x1ff) * 8);
            if(*tmp == 0)
            {
                virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
                memset(virtual, 0, PAGE_4K_SIZE);
                set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_KERNEL_DIR));
            }
            tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + (((unsigned long)P_TO_V(page->p_address) >> PAGE_2M_SHIFT) & 0x1ff) * 8);
            set_pdt(tmp, make_pdt(page->p_address, PAGE_KERNEL_PAGE));
        }
    }
    flush_tlb();
    return;
}

unsigned long do_brk(unsigned long addr, unsigned long len)
{
    unsigned long* tmp = NULL;
    unsigned long* virtual = NULL;
    struct Page* p = NULL;
    unsigned long i = 0;
    for(i = addr; i < addr + len; i += PAGE_2M_SIZE)
    {
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)current->mm->pgd & (~0xfffUL))) + ((i >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
        if(*tmp == 0)
        {
            virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
            memset(virtual, 0, PAGE_4K_SIZE);
            set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_USER_GDT));
        }
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((i >> PAGE_1G_SHIFT) & 0x1ff) * 8);
        if(*tmp == 0)
        {
            virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
            memset(virtual, 0, PAGE_4K_SIZE);
            set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_USER_DIR));
        }
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((i >> PAGE_2M_SHIFT) & 0x1ff) * 8);
        if(*tmp == 0)
        {
            p = alloc_pages(ZONE_NORMAL, 1, PAGE_PT_MAPED);
            memset(p, 0, PAGE_2M_SIZE);
            if(p == NULL)
            {
                return -ENOMEM;
            }
            set_pdt(tmp, make_pdt(p->p_address, PAGE_USER_PAGE));
        }
    }
    current->mm->end_brk = i;
    flush_tlb();
    return i;
}