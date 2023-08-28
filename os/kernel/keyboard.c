#include "keyboard.h"
#include "lib.h"
#include "printk.h"
#include "APIC.h"
#include "ptrace.h"
#include "memory.h"
#include "vfs.h"
#include "waitqueue.h"
#include "task.h"

struct keyboard_input_buffer *p_kb = NULL;
wait_queue_t keyboard_wait_queue;

int_controler keyboard_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};
void keyboard_exit()
{
    unregister_irq(0x21);
    kfree((unsigned long *)p_kb);
    return;
}
void keyboard_handler(unsigned long nr, unsigned long parameter, struct registers_in_stack *reg)
{
    unsigned char x = 0;
    x = io_in8(PORT_KB_DATA);
    if (x == 0xfa)
    {
        return;
    }
    if (p_kb->p_head == p_kb->buf + KEYBOARD_BUF_SIZE)
    {
        p_kb->p_head = p_kb->buf;
    }
    *p_kb->p_head = x;
    p_kb->count++;
    p_kb->p_head++;
    wakeup(&keyboard_wait_queue, TASK_UNINTERRUPTIBLE);
    return;
}
void keyboard_init()
{
    struct IOAPIC_RET_ENTRY entry;
    unsigned long i = 0;
    unsigned long j = 0;
    wait_queue_init(&keyboard_wait_queue, NULL);
    p_kb = (struct keyboard_input_buffer *)kmalloc(sizeof(struct keyboard_input_buffer), 0);
    if (p_kb == NULL)
    {
        color_printk(RED, BLACK, "kmalloc()->p_kb failed\n");
    }
    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KEYBOARD_BUF_SIZE);
    entry.vector_num = 0x21;
    entry.deliver_mode = IOAPIC_FIXED;
    entry.deliver_status = IOAPIC_DELI_STATUS_IDLE;
    entry.dest_mode = IOAPIC_DEST_MODE_PHYSICAL;
    entry.polarity = IOAPIC_POLARITY_HIGH;
    entry.mask = IOAPIC_MASK_MASKED;
    entry.irr = IOAPIC_IRR_RESET;
    entry.trigger = IOAPIC_TRIGGER_EDGE;
    entry.reserved = 0;
    entry.destination.physical.reserved1 = 0;
    entry.destination.physical.reserved2 = 0;
    entry.destination.physical.phy_dest = 0;
    wait_KB_write();
    io_out8(PORT_KB_CMD, KB_WRITE_CMD);
    wait_KB_write();
    io_out8(PORT_KB_DATA, KB_INIT_MODE);
    for (j = 0; j < 1000; j++)
    {
        for (i = 0; i < 1000; i++)
        {
            nop();
        }
    }
    color_printk(GREEN, BLACK, "register_irq 0x21 keyboard\n");
    register_irq(0x21, &entry, &keyboard_handler, (unsigned long)p_kb, &keyboard_int_controller, "ps/2 keyboard");
    return;
}

long keyboard_open(struct index_node *inode, struct file *filp)
{
    filp->private_data = p_kb;

    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KEYBOARD_BUF_SIZE);

    return 1;
}

long keyboard_close(struct index_node *inode, struct file *filp)
{
    filp->private_data = NULL;

    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KEYBOARD_BUF_SIZE);

    return 1;
}

#define KEY_CMD_RESET_BUFFER 0

long keyboard_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg)
{
    switch (cmd)
    {

    case KEY_CMD_RESET_BUFFER:
        p_kb->p_head = p_kb->buf;
        p_kb->p_tail = p_kb->buf;
        p_kb->count = 0;
        memset(p_kb->buf, 0, KEYBOARD_BUF_SIZE);
        break;

    default:
        break;
    }

    return 0;
}

long keyboard_read(struct file *filp, char *buf, unsigned long count, long *position)
{
    long counter = 0;
    unsigned char *tail = NULL;

    if (p_kb->count == 0)
    {
        sleep_on(&keyboard_wait_queue);
    }
    counter = (p_kb->count >= count) ? count : p_kb->count;
    tail = p_kb->p_tail;

    if (counter <= (p_kb->buf + KEYBOARD_BUF_SIZE - tail))
    {
        copy_to_user(tail, buf, counter);
        p_kb->p_tail += counter;
    }
    else
    {
        copy_to_user(tail, buf, (p_kb->buf + KEYBOARD_BUF_SIZE - tail));
        copy_to_user(p_kb->p_head, buf, counter - (p_kb->buf + KEYBOARD_BUF_SIZE - tail));
        p_kb->p_tail = p_kb->p_head + (counter - (p_kb->buf + KEYBOARD_BUF_SIZE - tail));
    }
    p_kb->count -= counter;

    return counter;
}

long keyboard_write(struct file *filp, char *buf, unsigned long count, long *position)
{
    return 0;
}

struct file_operations keyboard_fops =
    {
        .open = keyboard_open,
        .close = keyboard_close,
        .ioctl = keyboard_ioctl,
        .read = keyboard_read,
        .write = keyboard_write,
};