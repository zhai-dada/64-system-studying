#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"
#include "interrupt.h"
#include "task.h"
#include "cpu.h"
#if APIC
#include "APIC.h"
#else
#include "8259A.h"
#endif
#include "keyboard.h"
#include "smp.h"
#include "spinlock.h"
#include "time.h"
#include "HPET.h"
#include "softirq.h"
#include "schedule.h"
#include "semaphore.h"
#include "atomic.h"
#include "preempt.h"
#include "UEFI.h"
#include "disk.h"
#include "fat32.h"
#include "spinlock.h"

extern char _text, _etext;
extern char _data, _edata;
extern char _end, _bss;
extern char _erodata;
extern long global_pid;
extern spinlock_t smp_lock;
struct Global_Memory_Descriptor mem_structure = {{0}, 0};
void *tmp = NULL;
struct SLAB *slab = NULL;
int global_i = 0;
spinlock_t sequence_lock;
struct KERNEL_BOOT_PARAMETER_INFORMATION *boot_para_info = (struct KERNEL_BOOT_PARAMETER_INFORMATION *)0xffff800000060000;

void Start_Kernel(void)
{
    memset((void *)&_bss, 0, (unsigned long)&_end - (unsigned long)&_bss);
    memset(Pos.FB_addr, 0, Pos.FB_length);
    global_pid = 1;
    unsigned char buff[512];
    int i = 0;
    struct INT_CMD_REG icr_entry;
    unsigned char *ptr = NULL;
    init_printk();
    spinlock_init(&Pos.printk_lock);
    spinlock_init(&sequence_lock);
    mem_structure.start_code = (unsigned long)&_text;
    mem_structure.end_code = (unsigned long)&_etext;
    mem_structure.start_data = (unsigned long)&_data;
    mem_structure.end_data = (unsigned long)&_edata;
    mem_structure.start_brk = (unsigned long)&_end;
    mem_structure.end_rodata = (unsigned long)&_erodata;
    load_TR(10);
    set_tss64((unsigned int *)&init_tss[0], _stack_start_, _stack_start_, _stack_start_, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);
    color_printk(RED, BLACK, "system vector initing...\n");
    sys_vector_init();
    color_printk(RED, BLACK, "memory initing...\n");
    init_memory();
    color_printk(RED, BLACK, "cpu initing...\n");
    init_cpu();
    color_printk(RED, BLACK, "slab init...\n");
    slab_init();
    ptr = (unsigned char *)kmalloc(STACK_SIZE, 0) + STACK_SIZE;
    ((struct task_struct *)(ptr - STACK_SIZE))->cpu_id = 0;
    init_tss[0].ist1 = (unsigned long)ptr;
    init_tss[0].ist2 = (unsigned long)ptr;
    init_tss[0].ist3 = (unsigned long)ptr;
    init_tss[0].ist4 = (unsigned long)ptr;
    init_tss[0].ist5 = (unsigned long)ptr;
    init_tss[0].ist6 = (unsigned long)ptr;
    init_tss[0].ist7 = (unsigned long)ptr;
    VBE_buffer_init();
    pagetable_init();
    local_APIC_init();
    color_printk(RED, BLACK, "interrupt initing...\n");
#if APIC
    APIC_IOAPIC_init();
#else
    init_8259A();
#endif
    color_printk(RED, BLACK, "keyboard initing...\n");
    keyboard_init();
    color_printk(RED, BLACK, "disk initing...\n");
    disk_init();
    color_printk(RED, BLACK, "smp initing...\n");
    smp_init();
    color_printk(RED, BLACK, "schedule initing...\n");
    schedule_init();
    color_printk(RED, BLACK, "softirq initing...\n");
    softirq_init();
    icr_entry.vector_num = 0x00;
    icr_entry.deliver_mode = IOAPIC_INIT;
    icr_entry.dest_mode = IOAPIC_DEST_MODE_PHYSICAL;
    icr_entry.deliver_status = IOAPIC_DELI_STATUS_IDLE;
    icr_entry.res_1 = 0;
    icr_entry.level = ICR_LEVEL_DE_ASSERT;
    icr_entry.trigger = IOAPIC_TRIGGER_EDGE;
    icr_entry.res_2 = 0;
    icr_entry.dest_shorthand = ICR_ALL_EXCLUDE_SELF;
    icr_entry.res_3 = 0;
    icr_entry.destination.x2apic_destination = 0x00;
    wrmsr(0x830, *(unsigned long *)&icr_entry); // init
#ifdef BOCHS
    for (global_i = 0; global_i < 4;)
#else
    for (global_i = 1; global_i < 1; global_i++)
#endif
    {
        spinlock_lock(&smp_lock);
        ptr = (unsigned char *)kmalloc(STACK_SIZE, 0) + STACK_SIZE;
        if (ptr == NULL)
        {
            color_printk(RED, BLACK, "kmalloc NULL\n");
        }
        ((struct task_struct *)(ptr - STACK_SIZE))->cpu_id = global_i;
        _stack_start_ = (unsigned long)ptr + STACK_SIZE;
        memset(&init_tss[global_i], 0, sizeof(struct tss_struct));
        init_tss[global_i].rsp0 = _stack_start_;
        init_tss[global_i].rsp1 = _stack_start_;
        init_tss[global_i].rsp2 = _stack_start_;

        ptr = (unsigned char *)kmalloc(STACK_SIZE, 0) + STACK_SIZE;
        ((struct task_struct *)(ptr - STACK_SIZE))->cpu_id = global_i;
        init_tss[global_i].ist1 = (unsigned long)ptr;
        init_tss[global_i].ist2 = (unsigned long)ptr;
        init_tss[global_i].ist3 = (unsigned long)ptr;
        init_tss[global_i].ist4 = (unsigned long)ptr;
        init_tss[global_i].ist5 = (unsigned long)ptr;
        init_tss[global_i].ist6 = (unsigned long)ptr;
        init_tss[global_i].ist7 = (unsigned long)ptr;
        set_tss_descriptor(10 + global_i * 2, &init_tss[global_i]);
        icr_entry.vector_num = 0x20;
        icr_entry.deliver_mode = IOAPIC_ICR_START_UP;
        icr_entry.dest_shorthand = ICR_NO_SHORTHAND;
        icr_entry.destination.x2apic_destination = global_i;
        wrmsr(0x830, *(unsigned long *)&icr_entry);
        wrmsr(0x830, *(unsigned long *)&icr_entry);
        color_printk(YELLOW, BLACK, "Send Start-IPI End\n");
    }
    color_printk(GREEN, BLACK, "timer initing...\n");
    timer_init();
    color_printk(GREEN, BLACK, "timer clock initing...\n");
    HPET_init();
    color_printk(RED, BLACK, "task init\n");
    task_init();
    EOI();
    sti();
    while (TRUE)
    {
        hlt();
    }
    return;
}
