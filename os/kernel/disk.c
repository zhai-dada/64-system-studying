#include "disk.h"
#include "memory.h"
#include "semaphore.h"
#include "printk.h"
#include "APIC.h"
#include "interrupt.h"
#include "lib.h"


struct request_queue disk_request;
unsigned long cmd_out()
{
    wait_queue_t *wait_tmp = container_of(list_next(&disk_request.wait_queue_list.wait_list), wait_queue_t, wait_list);
    struct block_buffer_node *node = disk_request.in_using = container_of(wait_tmp, struct block_buffer_node, wait_queue);
    list_delete(&disk_request.in_using->wait_queue.wait_list);
    disk_request.block_request_count--;
    while (io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_BUSY)
    {
        nop();
    }
    switch (node->cmd)
    {
    case ATA_WRITE_CMD:
        io_out8(PORT_DISK0_DEVICE, 0xe0 + ((node->LBA >> 24) & 0xf));

        io_out8(PORT_DISK0_ERR_FEATURE, 0);
        io_out8(PORT_DISK0_SECTOR_CNT, node->count & 0xff);
        io_out8(PORT_DISK0_SECTOR_LOW, node->LBA & 0xff);
        io_out8(PORT_DISK0_SECTOR_MID, (node->LBA >> 8) & 0xff);
        io_out8(PORT_DISK0_SECTOR_HIGH, (node->LBA >> 16) & 0xff);
        while (!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY))
        {
            nop();
        }
        io_out8(PORT_DISK0_STATUS_CMD, ATA_WRITE_CMD);
        while (!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ))
        {
            nop();
        }
        port_outsw(PORT_DISK0_DATA, node->buffer, 256);
        break;
    case ATA_READ_CMD:
        io_out8(PORT_DISK0_DEVICE, 0xe0 + ((node->LBA >> 24) & 0xf));

        io_out8(PORT_DISK0_ERR_FEATURE, 0);
        io_out8(PORT_DISK0_SECTOR_CNT, node->count & 0xff);
        io_out8(PORT_DISK0_SECTOR_LOW, node->LBA & 0xff);
        io_out8(PORT_DISK0_SECTOR_MID, (node->LBA >> 8) & 0xff);
        io_out8(PORT_DISK0_SECTOR_HIGH, (node->LBA >> 16) & 0xff);
        while (!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_READY))
        {
            nop();
        }
        io_out8(PORT_DISK0_STATUS_CMD, ATA_READ_CMD);
        break;
    default:
        color_printk(RED, WHITE, "ATA CMD Error\n");
        break;
    }
}
void end_request(struct block_buffer_node *node)
{
    if(node == NULL)
    {
        color_printk(RED, BLACK, "end_request error\n");
    }
    node->wait_queue.task->state = TASK_RUNNING;
    insert_task_queue(node->wait_queue.task);
    current->flags |= NEED_SHEDULE;
    kfree((unsigned long *)disk_request.in_using);
    disk_request.in_using = NULL;
    if (disk_request.block_request_count)
    {
        cmd_out();
    }
    return;
}
void write_handler(unsigned long nr, unsigned long parameter)
{
    struct block_buffer_node *node = ((struct request_queue *)parameter)->in_using;
    if (io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
    {
        color_printk(RED, YELLOW, "write_handler:%#010x\n", io_in8(PORT_DISK0_ERR_FEATURE));
    }
    node->count--;
    if(node->count)
    {
        node->buffer += 512;
        while(!(io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_REQ))
        {
            nop();
        }
        port_outsw(PORT_DISK0_DATA, node->buffer, 256);
        return;
    }
    end_request(node);
    return;
}
void other_handler(unsigned long nr, unsigned long parameter)
{
    struct block_buffer_node *node = ((struct request_queue *)parameter)->in_using;
    if (io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
    {
        color_printk(RED, YELLOW, "other_handler:%#010x\n", io_in8(PORT_DISK0_ERR_FEATURE));
    }
    end_request(node);
}
void read_handler(unsigned long nr, unsigned long parameter)
{
    struct block_buffer_node *node = ((struct request_queue *)parameter)->in_using;
    if (io_in8(PORT_DISK0_STATUS_CMD) & DISK_STATUS_ERROR)
    {
        color_printk(RED, YELLOW, "read_handler:%#010x\n", io_in8(PORT_DISK0_ERR_FEATURE));
    }
    else
    {
        port_insw(PORT_DISK0_DATA, node->buffer, 256);
    }
    node->count--;
    if(node->count)
    {
        node->buffer += 512;
        return;
    }
    end_request(node);
    return;
}
struct block_buffer_node *make_request(unsigned long cmd, unsigned long bloks, unsigned long count, unsigned char *buffer)
{
    struct block_buffer_node *node = (struct block_buffer_node *)kmalloc(sizeof(struct block_buffer_node), 0);
    wait_queue_init(&node->wait_queue, current);
    switch (cmd)
    {
    case ATA_READ_CMD:
        node->end_handler = read_handler;
        node->cmd = ATA_READ_CMD;
        break;

    case ATA_WRITE_CMD:
        node->end_handler = write_handler;
        node->cmd = ATA_WRITE_CMD;
        break;
    default:
        node->end_handler = other_handler;
        node->cmd = cmd;
        break;
    }
    node->LBA = bloks;
    node->count = count;
    node->buffer = buffer;
    return node;
}
void add_request(struct block_buffer_node *node)
{
    list_add_before(&disk_request.wait_queue_list.wait_list, &node->wait_queue.wait_list);
    disk_request.block_request_count++;
    return;
}

void submit(struct block_buffer_node *node)
{
    add_request(node);
    if (disk_request.in_using == NULL)
    {
        cmd_out();
    }
    return;
}
void wait_for_finish()
{
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
    return;
}
unsigned long disk_transfer(unsigned long cmd, unsigned long bloks, unsigned long count, unsigned char *buffer)
{
    struct block_buffer_node *node = NULL;
    if (cmd == ATA_READ_CMD || cmd == ATA_WRITE_CMD)
    {
        node = make_request(cmd, bloks, count, buffer);
        submit(node);
        wait_for_finish();
    }
    else
    {
        return 0;
    }
    return 1;
}
int_controler disk_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack};
struct block_device_operation disk_device_operation =
    {
        .open = NULL,
        .close = NULL,
        .ioctl = NULL,
        .transfer = disk_transfer};
void disk_handler(unsigned long nr, unsigned long parameter, struct registers_in_stack *reg)
{
    struct block_buffer_node *node = ((struct request_queue *)parameter)->in_using;
    if (node != NULL)
        node->end_handler(nr, parameter);
    return;
}
void disk_init()
{
    struct IOAPIC_RET_ENTRY entry;
    entry.vector_num = 0x2e;
    entry.deliver_mode = IOAPIC_FIXED;
    entry.dest_mode = IOAPIC_DEST_MODE_PHYSICAL;
    entry.deliver_status = IOAPIC_DELI_STATUS_IDLE;
    entry.polarity = IOAPIC_POLARITY_HIGH;
    entry.irr = IOAPIC_IRR_RESET;
    entry.trigger = IOAPIC_TRIGGER_EDGE;
    entry.mask = IOAPIC_MASK_MASKED;
    entry.reserved = 0;
    entry.destination.physical.reserved1 = 0;
    entry.destination.physical.reserved2 = 0;
    entry.destination.physical.phy_dest = 0;
    register_irq(0x2e, &entry, &disk_handler, (unsigned long)&disk_request, &disk_controller, "disk0");
    io_out8(PORT_DISK0_ALT_STA_CTL, 0);

    wait_queue_init(&disk_request.wait_queue_list, NULL);
    disk_request.in_using = NULL;
    disk_request.block_request_count = 0;

    return;
}