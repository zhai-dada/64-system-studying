#ifndef __DISK_H__
#define __DISK_H__
#include "semaphore.h"
#include "waitqueue.h"

#define PORT_DISK0_DATA			0x1f0
#define	PORT_DISK0_ERR_FEATURE	0x1f1
#define	PORT_DISK0_SECTOR_CNT	0x1f2
#define	PORT_DISK0_SECTOR_LOW	0x1f3
#define	PORT_DISK0_SECTOR_MID	0x1f4
#define	PORT_DISK0_SECTOR_HIGH	0x1f5
#define	PORT_DISK0_DEVICE		0x1f6
#define	PORT_DISK0_STATUS_CMD	0x1f7

#define	PORT_DISK0_ALT_STA_CTL	0x3f6

#define PORT_DISK1_DATA			0x170
#define	PORT_DISK1_ERR_FEATURE	0x171
#define	PORT_DISK1_SECTOR_CNT	0x172
#define	PORT_DISK1_SECTOR_LOW	0x173
#define	PORT_DISK1_SECTOR_MID	0x174
#define	PORT_DISK1_SECTOR_HIGH	0x175
#define	PORT_DISK1_DEVICE		0x176
#define	PORT_DISK1_STATUS_CMD	0x177

#define	PORT_DISK1_ALT_STA_CTL	0x376

#define	DISK_STATUS_BUSY		(1 << 7)
#define	DISK_STATUS_READY		(1 << 6)
#define	DISK_STATUS_SEEK		(1 << 4)
#define	DISK_STATUS_REQ			(1 << 3)
#define	DISK_STATUS_ERROR		(1 << 0)

#define ATA_READ_CMD		0x20
#define ATA_WRITE_CMD		0x30

struct block_buffer_node
{
    unsigned int count;
    unsigned char cmd;
    unsigned long LBA;
    unsigned char* buffer;
    void (*end_handler)(unsigned long nr, unsigned long parameter);
    wait_queue_t wait_queue;
};
struct request_queue
{
    wait_queue_t wait_queue_list;
    struct block_buffer_node* in_using;
    unsigned long block_request_count;
};
struct block_device_operation
{
    unsigned long (*open)();
    unsigned long (*close)();
    unsigned long (*ioctl)(unsigned long cmd, unsigned long arg);
    unsigned long (*transfer)(unsigned long cmd, unsigned long blocks, unsigned long count, unsigned char* buffer);
};
extern struct block_device_operation disk_device_operation;
extern struct request_queue disk_request;
extern unsigned long disk_flags;
void disk_init(void);

#endif