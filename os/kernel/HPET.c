#include "time.h"
#include "APIC.h"
#include "interrupt.h"
#include "printk.h"
#include "lib.h"
#include "memory.h"
#include "HPET.h"
#include "softirq.h"
#include "task.h"
#include "schedule.h"
#include "smp.h"

extern struct time time;

int_controler HPET_controller = 
{
    .enable = IOAPIC_enable,
    .disable = IOAPIC_disable,
    .install = IOAPIC_install,
    .uninstall = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack,
};
void HPET_handler(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg)
{
    jiffies++;
    if(container_of(list_next(&timer_list_head.list), struct timer_list, list)->expire_jiffies <= jiffies)
    {
        set_softirq_status(TIMER_S_IRQ);
    }
    struct INT_CMD_REG icr_entry;
    memset(&icr_entry, 0, sizeof(struct INT_CMD_REG));
    switch(current->priority)
    {
        case 0:
        case 1:
            task_schedule[smp_cpu_id()].CPU_exectask_jiffies -= 1;
            current->vruntime += 1;
            break;
        case 2:
        default:
            task_schedule[smp_cpu_id()].CPU_exectask_jiffies -= 2;
            current->vruntime += 2;
            break;
    }
    if(task_schedule[smp_cpu_id()].CPU_exectask_jiffies <= 0)
    {
        current->flags |= NEED_SHEDULE;
    }
    icr_entry.vector_num = 0xc8;
    icr_entry.dest_shorthand = ICR_ALL_EXCLUDE_SELF;
    icr_entry.trigger = IOAPIC_TRIGGER_EDGE;
    icr_entry.dest_mode = IOAPIC_DEST_MODE_PHYSICAL;
    icr_entry.deliver_mode = IOAPIC_FIXED;
    wrmsr(0x830, *(unsigned long*)&icr_entry);
    return;
}
void HPET_init()
{
    unsigned char* HPET_addr = (unsigned char*)P_TO_V(0xfed00000);
    struct IOAPIC_RET_ENTRY entry;
    entry.vector_num = 0x22;
    entry.deliver_mode = IOAPIC_FIXED;
    entry.dest_mode = IOAPIC_DEST_MODE_PHYSICAL;
    entry.deliver_status = IOAPIC_DELI_STATUS_IDLE;
    entry.irr = IOAPIC_IRR_RESET;
    entry.trigger = IOAPIC_TRIGGER_EDGE;
    entry.polarity = IOAPIC_POLARITY_HIGH;
    entry.mask = IOAPIC_MASK_MASKED;
    entry.reserved = 0;
    entry.destination.physical.reserved1 = 0;
    entry.destination.physical.reserved2 = 0;
    entry.destination.physical.phy_dest = 0;
    register_irq(0x22, &entry, &HPET_handler, NULL, &HPET_controller, "HPET");
    color_printk(YELLOW, BLACK, "HPET GCAP_ID:%#018lx\n", *(unsigned long*)(HPET_addr));
    *(unsigned long*)(HPET_addr + 0x10) = 3;
    io_mfence();
    *(unsigned long*)(HPET_addr + 0x100) = 0x004c;
    io_mfence();
    *(unsigned long*)(HPET_addr + 0x108) = 14318179 * 1; // 1 s
    io_mfence();
    get_cmos_time(&time);
    *(unsigned long*)(HPET_addr + 0xf0) = 0;
    io_mfence();
    color_printk(YELLOW, BLACK, "%x:%x:%x:%x:%x:%x\n", time.year, time.month, time.day, time.hour, time.minute, time.second);
    return;
}