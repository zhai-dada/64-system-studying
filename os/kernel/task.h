#ifndef __TASK_H__
#define __TASK_H__

#include "linkage.h"
#include "lib.h"
#include "cpu.h"
#include "printk.h"
#include "smp.h"
#include "memory.h"
#include "vfs.h"
#include "sched.h"
#include "ptrace.h"

#define STACK_SIZE 32768
#define TASK_SIZE 0x00007fffffffffff
#define KERNEL_CODE_SEGMENT (0x08)
#define KERNEL_DATA_SEGMENT (0x10)
#define	USER_CODE_SEGMENT	(0x28)
#define USER_DATA_SEGMENT 	(0x30)


extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;
extern long global_pid;

extern unsigned long _stack_start_;
extern void ret_from_intr();

struct mm_struct
{
    pml4t_t* pgd;
    unsigned long start_code, end_code;
    unsigned long start_data, end_data;
    unsigned long start_rodata, end_rodata;
    unsigned long start_brk, end_brk;
    unsigned long start_bss, end_bss;
    unsigned long start_stack;
};
struct thread_struct
{
    unsigned long rsp0;
    unsigned long rip;
    unsigned long rsp;
    unsigned long fs;
    unsigned long gs;
    unsigned long cr2;
    unsigned long trap_nr;
    unsigned long error_code;
};

#define PF_KTHREAD  (1UL << 0)
#define NEED_SHEDULE    (1UL << 1)
#define PF_VFORK	(1UL << 2)

#define TASK_FILE_MAX 10
struct task_struct
{
    volatile long state;    //0x00
    unsigned long flags;    //0x08
    long preempt_count;     //0x10
    long signal;            //0x18
    long cpu_id;
    long pid;               
    long priority;          
    long vruntime;          
    unsigned long addr_limit;
    struct mm_struct* mm;
    struct thread_struct* thread;
    struct List list;
    struct file* filestruct[TASK_FILE_MAX];
    struct task_struct *next;
	struct task_struct *parent;
};

union task_stack_union
{
    struct task_struct task;
    unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
}__attribute__((aligned(8)));

#define TASK_RUNNING            (1UL << 0)
#define TASK_INTERRUPTIBLE      (1UL << 1)
#define TASK_UNINTERRUPTIBLE    (1UL << 2)
#define TASK_ZOMBIE             (1UL << 3)
#define TASK_STOPPED            (1UL << 4)

extern struct mm_struct init_mm;
extern struct thread_struct init_thread;

#define INIT_TASK(task)                     \
{                                           \
    .state = TASK_UNINTERRUPTIBLE,          \
    .flags = PF_KTHREAD,                    \
    .preempt_count = 0,                         \
    .signal = 0,                            \
    .cpu_id = 0,                                \
    .pid = 0,                               \
    .priority = 2,                           \
    .vruntime = 0,                           \
    .addr_limit = 0xffff800000000000,       \
    .mm = &init_mm,                         \
    .thread = &init_thread,               \
    .filestruct = {0},                       \
    .next = &task,                          \
    .parent = &task                        \
}

struct tss_struct
{
    unsigned int reserved0;
    unsigned long rsp0;
    unsigned long rsp1;;
    unsigned long rsp2;
    unsigned long reserved1;
    unsigned long ist1;
    unsigned long ist2;
    unsigned long ist3;
    unsigned long ist4;
    unsigned long ist5;
    unsigned long ist6;
    unsigned long ist7;
    unsigned long reserved2;
    unsigned short reserved3;
    unsigned short iomapbaseaddr;
}PACKED;
#define INIT_TSS                \
{                               \
    .reserved0 = 0,             \
    .rsp0 = (unsigned long)(init_task_stack.stack + STACK_SIZE / sizeof(unsigned long)),  \
    .rsp1 = (unsigned long)(init_task_stack.stack + STACK_SIZE / sizeof(unsigned long)),  \
    .rsp2 = (unsigned long)(init_task_stack.stack + STACK_SIZE / sizeof(unsigned long)),  \
    .reserved1 = 0,             \
    .ist1 = 0xffff800000007c00, \
    .ist2 = 0xffff800000007c00, \
    .ist3 = 0xffff800000007c00, \
    .ist4 = 0xffff800000007c00, \
    .ist5 = 0xffff800000007c00, \
    .ist6 = 0xffff800000007c00, \
    .ist7 = 0xffff800000007c00, \
    .reserved2 = 0,             \
    .reserved3 = 0,             \
    .iomapbaseaddr = 0          \
}

struct task_struct* get_current()
{
    struct task_struct* current = NULL;
    asm volatile
    (
        "andq %%rsp, %0\n"
        :"=r"(current)
        :"0"(~32767UL)
        :
    );
    return current;
}
#define current get_current()
#define GET_CURRENT             \
    "movq %rsp, %rbx\n"         \
    "andq $-32768, %rbx\n"

#define switch_to(prev, next)                   \
do                                              \
{                                               \
    asm volatile                                \
    (                                           \
        "pushq %%rbp\n"                         \
        "pushq %%rax\n"                         \
        "movq %%rsp, %0\n"                      \
        "movq %2, %%rsp\n"                      \
        "leaq 1f(%%rip), %%rax\n"               \
        "movq %%rax, %1\n"                      \
        "pushq %3\n"                            \
        "jmp __switch_to__\n"                   \
        "1:\n"                                  \
        "popq %%rax\n"                          \
        "popq %%rbp\n"                          \
        :"=m"(prev->thread->rsp), "=m"(prev->thread->rip)  \
        :"m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next)  \
        :"memory"                               \
    );                                          \
} while (0);

extern union task_stack_union init_task_stack __attribute__((__section__(".data.init_task_stack")));
extern struct task_struct* init_task[NR_CPUS];
extern struct mm_struct init_mm;
extern struct tss_struct init_tss[NR_CPUS];

void task_init(void);
void __switch_to__(struct task_struct* prev, struct task_struct* next);
void switch_mm(struct task_struct *prev, struct task_struct *next);
unsigned long do_fork(struct registers_in_stack *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);
void wakeup_process(struct task_struct *task);
#endif