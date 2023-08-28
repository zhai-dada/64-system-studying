#include "smp.h"
#include "memory.h"
#include "printk.h"
#include "APIC.h"
#include "interrupt.h"
#include "lib.h"
#include "cpu.h"
#include "gate.h"
#include "task.h"
#include "schedule.h"

extern int global_i;
spinlock_t smp_lock;
void ipi_200(unsigned long nr, unsigned long parameter, struct registers_in_stack* regs);
void smp_init()
{
    int i = 0;
    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;
    for(i = 0; ; i++)
    {
        cpuid(0x0b, i, &a, &b, &c, &d);
        if(((c >> 8) & 0xff) == 0)
        {
            break;
        }
        color_printk(ORANGE, BLACK, "local apic id smt/core (type:%x) width:%#010x, num of logical processor(%x)\n", (c >> 8) & 0xff, a & 0x1f, b & 0xff);
    }
    color_printk(ORANGE, BLACK, "x2APIC ID level:%#010x\tcurrent x2APIC ID:%#010x\n", c & 0xff, d);
    color_printk(YELLOW, BLACK, "SMP copy byte:%#010lx\n", (unsigned long)&APU_BOOT_END - (unsigned long)&APU_BOOT_START);
    memcpy(APU_BOOT_START, (unsigned char*)0xffff800000020000, (unsigned long)&APU_BOOT_END - (unsigned long)&APU_BOOT_START);
    spinlock_init(&smp_lock);
    for(i = 0xc8; i <= 0xd1; i++)
    {
        set_intr_gate(i, 2, smp_interrupt[i - 0xc8]);
    }
    memset(SMP_IPI_desc, 0, sizeof(int_desc_t) * 10);
    register_ipi(200, NULL, &ipi_200, NULL, NULL, "IPI 200");
    return;
}
void start_smp()
{
    unsigned int x = 0;
    unsigned int y = 0;
    color_printk(GREEN, BLACK, "APU starting....\n");
    asm volatile
    (
        "movq $0x1b, %%rcx\n"
        "rdmsr\n"
        "bts $10, %%rax\n"
        "bts $11, %%rax\n"
        "wrmsr\n"
        "movq $0x1b, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    if(x & 0x0c00)
    {
        color_printk(WHITE, BLACK, "xAPIC & x2APIC enabled\n");
    }
    asm volatile
    (
        "movq $0x80f, %%rcx\n"
        "rdmsr\n"
        "bts $8, %%rax\n"
        #ifndef BOCHS
        "bts $12, %%rax\n"
        #endif
        "wrmsr\n"
        "movq $0x80f, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    if(x & 0x100)
    {
        color_printk(WHITE, BLACK, "SVR[8] enabled\t");
    }
    if(x & 0x1000)
    {
        color_printk(WHITE, BLACK, "SVR[12] enabled");
    }
    asm volatile
    (
        "movq $0x802, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(WHITE, BLACK, "x2APIC ID:%#010x\n", x);
    color_printk(YELLOW, BLACK, "CPU: %d\n", smp_cpu_id());
    current->state = TASK_RUNNING;
    current->flags = PF_KTHREAD;
    current->mm = &init_mm;
    list_init(&current->list);
    current->addr_limit = 0xffff800000000000;
    current->priority = 2;
    current->vruntime = 0;
    current->thread = (struct thread_struct*)(current + 1);
    memset(current->thread, 0, sizeof(struct thread_struct));
    current->thread->rsp0 = _stack_start_;
    current->thread->rsp = _stack_start_;
    current->thread->fs = KERNEL_DATA_SEGMENT;
    current->thread->gs = KERNEL_DATA_SEGMENT;
    init_task[smp_cpu_id()] = current;
    load_TR(10 + smp_cpu_id() * 2);
    spinlock_unlock(&smp_lock);
    current->preempt_count = 0;
    sti();
    // task_init();
    while(TRUE)
    {
        nop();
        hlt();
    }
    return;
}

void ipi_200(unsigned long nr, unsigned long parameter, struct registers_in_stack* regs)
{
    switch(current->priority)
    {
        case 0:
        case 1:
            task_schedule[smp_cpu_id()].CPU_exectask_jiffies--;
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
    return;
}