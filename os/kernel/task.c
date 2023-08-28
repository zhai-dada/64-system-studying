#include "task.h"
#include "ptrace.h"
#include "gate.h"
#include "schedule.h"
#include "fat32.h"
#include "unistd.h"
#include "vfs.h"
#include "errno.h"
#include "fcntl.h"
extern spinlock_t sequence_lock;
struct mm_struct init_mm;
long global_pid;
struct thread_struct init_thread;
struct thread_struct init_thread =
    {
        .rsp0 = (unsigned long)(init_task_stack.stack + STACK_SIZE / sizeof(unsigned long)),
        .rsp = (unsigned long)(init_task_stack.stack + STACK_SIZE / sizeof(unsigned long)),
        .fs = KERNEL_DATA_SEGMENT,
        .gs = KERNEL_DATA_SEGMENT,
        .cr2 = 0,
        .trap_nr = 0,
        .error_code = 0};
union task_stack_union init_task_stack __attribute__((__section__(".data.init_task_stack"))) = {INIT_TASK(init_task_stack.task)};
struct task_struct *init_task[NR_CPUS] = {&init_task_stack.task, 0};
struct mm_struct init_mm = {0};
struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};

struct task_struct *get_task(long pid)
{
    struct task_struct *task = NULL;
    for (task = init_task_stack.task.next; task != &init_task_stack.task; task = task->next)
    {
        if (task->pid == pid)
        {
            return task;
        }
    }
    return NULL;
}
struct file *open_exec_file(char *path)
{
    struct dir_entry *dentry = NULL;
    struct file *filp = NULL;

    dentry = path_walk(path, 0);

    if (dentry == NULL)
    {
        return (struct file *)-ENOENT;
    }
    if (dentry->dir_inode->attribute == FS_ATTR_DIR)
    {
        return (struct file *)-ENOTDIR;
    }
    filp = (struct file *)kmalloc(sizeof(struct file), 0);
    if (filp == NULL)
    {
        return (struct file *)-ENOMEM;
    }
    filp->position = 0;
    filp->mode = 0;
    filp->dentry = dentry;
    filp->mode = O_RDONLY;
    filp->f_ops = dentry->dir_inode->f_ops;

    return filp;
}
extern void ret_system_call(void);
extern void fc(void);

inline void wakeup_process(struct task_struct *task)
{
    task->state = TASK_RUNNING;
    insert_task_queue(task);
    current->flags |= NEED_SHEDULE;
    return;
}
unsigned long copy_flags(unsigned long clone_flags, struct task_struct *task)
{
    if (clone_flags & CLONE_VM)
    {
        task->flags |= PF_VFORK;
    }
    return 0;
}
unsigned long copy_file(unsigned long clone_flags, struct task_struct *task)
{
    int error = 0;
    int i = 0;
    if (clone_flags & CLONE_FS)
    {
        goto out;
    }
    for (i = 0; i < TASK_FILE_MAX; i++)
    {
        if (current->filestruct[i] != NULL)
        {
            task->filestruct[i] = (struct file *)kmalloc(sizeof(struct file), 0);
            memcpy(current->filestruct[i], task->filestruct[i], sizeof(struct file));
        }
    }
out:
    return error;
}
void exit_file(struct task_struct *task)
{
    int i = 0;
    if (task->flags & PF_VFORK)
    {
        return;
    }
    else
    {
        for (i = 0; i < TASK_FILE_MAX; i++)
        {
            if (task->filestruct[i] != NULL)
            {
                kfree(task->filestruct[i]);
            }
        }
    }
    memset(task->filestruct, 0, sizeof(struct file *) * TASK_FILE_MAX);
    return;
}
unsigned long copy_mem(unsigned long clone_flags, struct task_struct *task)
{
    int error = 0;
    struct mm_struct *newmm = NULL;
    unsigned long code_start_addr = 0x800000;
    unsigned long stack_start_addr = 0xa00000;
    unsigned long brk_start_addr = 0xc00000;
    unsigned long *tmp;
    unsigned long *virtual = NULL;
    struct Page *p = NULL;

    if (clone_flags & CLONE_VM)
    {
        newmm = current->mm;
        goto out;
    }

    newmm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
    memset(newmm, 0, sizeof(struct mm_struct));
    memcpy(current->mm, newmm, sizeof(struct mm_struct));

    newmm->pgd = (pml4t_t *)V_TO_P(kmalloc(PAGE_4K_SIZE, 0));
    memcpy((void *)(P_TO_V(init_task[smp_cpu_id()]->mm->pgd) + 256 * 8), (void *)(P_TO_V(newmm->pgd) + 256 * 8), PAGE_4K_SIZE / 2); // copy kernel space

    memset((void *)(P_TO_V(newmm->pgd)), 0, PAGE_4K_SIZE / 2); // copy user code & data & bss space

    tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)newmm->pgd & (~0xfffUL))) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
    virtual = kmalloc(PAGE_4K_SIZE, 0);
    memset(virtual, 0, PAGE_4K_SIZE);
    set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_USER_GDT));

    tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff) * 8);
    virtual = kmalloc(PAGE_4K_SIZE, 0);
    memset(virtual, 0, PAGE_4K_SIZE);
    set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_USER_DIR));

    tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff) * 8);
    p = alloc_pages(ZONE_NORMAL, 1, PAGE_PT_MAPED);
    set_pdt(tmp, make_pdt(p->p_address, PAGE_USER_PAGE));

    memcpy((void *)(code_start_addr), (void *)(P_TO_V(p->p_address)), stack_start_addr - code_start_addr);
    if (current->mm->end_brk - current->mm->start_brk != 0)
    {
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)newmm->pgd & (~0xfffUL))) + ((brk_start_addr >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((brk_start_addr >> PAGE_1G_SHIFT) & 0x1ff) * 8);
        tmp = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp) & (~0xfffUL))) + ((brk_start_addr >> PAGE_2M_SHIFT) & 0x1ff) * 8);
        p = alloc_pages(ZONE_NORMAL, 5, PAGE_PT_MAPED);
        set_pdt(tmp, make_pdt(p->p_address, PAGE_USER_PAGE));
        memcpy((void *)brk_start_addr, (void *)P_TO_V(p->p_address), PAGE_2M_SIZE * 5);
    }

out:
    task->mm = newmm;
    return error;
}
void exit_mem(struct task_struct *task)
{
    unsigned long code_start_addr = 0x800000;
    unsigned long *tmp4;
    unsigned long *tmp3;
    unsigned long *tmp2;

    if (task->flags & PF_VFORK)
    {
        return;
    }
    if (task->mm->pgd != NULL)
    {
        tmp4 = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)task->mm->pgd & (~0xfffUL))) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
        tmp3 = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp4) & (~0xfffUL))) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff) * 8);
        tmp2 = (unsigned long *)(((unsigned long)P_TO_V((unsigned long)(*tmp3) & (~0xfffUL))) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff) * 8);

        free_pages(P_TO_2M(*tmp2), 1);
        kfree((void *)(P_TO_V(*tmp3)));
        kfree((void *)(P_TO_V(*tmp4)));
        kfree((void *)(P_TO_V(task->mm->pgd)));
    }
    if (task->mm != NULL)
    {
        kfree(task->mm);
    }
}
void switch_mm(struct task_struct *prev, struct task_struct *next)
{
    asm volatile(
        "movq %0, %%cr3	\n"
        :
        : "r"(next->mm->pgd)
        : "memory");
    return;
}
unsigned long copy_thread(unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size, struct task_struct *task, struct registers_in_stack *regs)
{
    struct thread_struct *thd = NULL;
    struct registers_in_stack *childregs = NULL;

    thd = (struct thread_struct *)(task + 1);
    memset(thd, 0, sizeof(struct thread_struct));
    task->thread = thd;

    childregs = (struct registers_in_stack *)((unsigned long)task + STACK_SIZE) - 1;

    memcpy(regs, childregs, sizeof(struct registers_in_stack));
    childregs->rax = 0;
    childregs->rsp = stack_start;

    thd->rsp0 = (unsigned long)task + STACK_SIZE;
    thd->rsp = (unsigned long)childregs;
    thd->fs = current->thread->fs;
    thd->gs = current->thread->gs;

    if (task->flags & PF_KTHREAD)
    {
        thd->rip = (unsigned long)fc;
    }
    else
    {
        thd->rip = (unsigned long)ret_system_call;
    }
    color_printk(WHITE, BLACK, "current user ret addr:%#018lx,rsp:%#018lx\n", regs->r10, regs->r11);
    color_printk(WHITE, BLACK, "new user ret addr:%#018lx,rsp:%#018lx\n", childregs->r10, childregs->r11);

    return 0;
}
void exit_thread(struct task_struct *task)
{
    return;
}
unsigned long do_fork(struct registers_in_stack *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size)
{
    int retval = 0;
    struct task_struct *task = NULL;
    task = (struct task_struct *)kmalloc(STACK_SIZE, 0);
    color_printk(WHITE, BLACK, "struct task_struct address:%#018lx\n", (unsigned long)task);
    if (task == NULL)
    {
        retval = -EAGAIN;
        goto alloc_copy_task_fail;
    }
    memset(task, 0, sizeof(struct task_struct));
    memcpy(current, task, sizeof(struct task_struct));
    list_init(&task->list);
    task->priority = 2;
    task->cpu_id = smp_cpu_id();
    task->pid = global_pid++;
    task->preempt_count = 0;
    task->state = TASK_UNINTERRUPTIBLE;
    task->next = current->next;
    current->next = task;
    task->parent = current;
    retval = -ENOMEM;
    if (copy_flags(clone_flags, task))
    {
        goto copy_flags_fail;
    }
    if (copy_mem(clone_flags, task))
    {
        goto copy_mem_fail;
    }
    if (copy_file(clone_flags, task))
    {
        goto copy_file_fail;
    }
    if (copy_thread(clone_flags, stack_start, stack_size, task, regs))
    {
        goto copy_thread_fail;
    }
    retval = task->pid;
    wakeup_process(task);
fork_ok:
    return retval;
copy_thread_fail:
    exit_thread(task);
copy_file_fail:
    exit_file(task);
copy_mem_fail:
    exit_mem(task);
copy_flags_fail:
alloc_copy_task_fail:
    kfree(task);
    return retval;
}
void __switch_to__(struct task_struct *prev, struct task_struct *next)
{
    unsigned int color = 0;
    init_tss[smp_cpu_id()].rsp0 = next->thread->rsp0;
    asm volatile("movq %%fs, %0\n"
                 : "=a"(prev->thread->fs));
    asm volatile("movq %%gs, %0\n"
                 : "=a"(prev->thread->gs));
    asm volatile("movq %0, %%fs\n" ::"a"(next->thread->fs));
    asm volatile("movq %0, %%gs\n" ::"a"(next->thread->gs));
    wrmsr(0x175, next->thread->rsp0);
    switch (smp_cpu_id())
    {
    case 0:
        color = GREEN;
        break;
    case 1 ... 3:
        color = ORANGE;
        break;
    case 4 ... 7:
        color = YELLOW;
        break;
    }
    // color_printk(color, BLACK, "prev->thread->rsp0:%#018lx\t", prev->thread->rsp0);
    // color_printk(color, BLACK, "prev->thread->rsp:%#018lx\n", prev->thread->rsp);
    // color_printk(color, BLACK, "next->thread->rsp0:%#018lx\t", next->thread->rsp0);
    // color_printk(color, BLACK, "next->thread->rsp:%#018lx\n", next->thread->rsp);
    // color_printk(color, BLACK, "CPUID:%#018lx\n", smp_cpu_id());
    return;
}

asm(
    ".global fc\n"
    "fc:\n"
    "popq %r15\n"
    "popq %r14\n"
    "popq %r13\n"
    "popq %r12\n"
    "popq %r11\n"
    "popq %r10\n"
    "popq %r9\n"
    "popq %r8\n"
    "popq %rbx\n"
    "popq %rcx\n"
    "popq %rdx\n"
    "popq %rsi\n"
    "popq %rdi\n"
    "popq %rbp\n"
    "popq %rax\n"
    "movq %rax, %ds\n"
    "popq %rax\n"
    "movq %rax, %es\n"
    "popq %rax\n"
    "addq $56, %rsp\n"
    "movq %rdx, %rdi\n"
    "callq *%rbx\n"
    "movq %rax, %rdi\n"
    "callq do_exit\n");
unsigned long do_exit(unsigned long code)
{
    color_printk(RED, BLACK, "exit task is running, arg code = %#018lx\n", code);
    while (true)
    {
        nop();
    }
    return 0;
}
int kernel_thread(unsigned long (*func)(unsigned long), unsigned long args, unsigned long flags)
{
    struct registers_in_stack regs;
    memset(&regs, 0, sizeof(regs));
    regs.rbx = (unsigned long)func;
    regs.rdx = args;
    regs.ds = KERNEL_DATA_SEGMENT;
    regs.es = KERNEL_DATA_SEGMENT;
    regs.cs = KERNEL_CODE_SEGMENT;
    regs.ss = KERNEL_DATA_SEGMENT;
    regs.rflags = (1 << 9);
    regs.rip = (unsigned long)fc;
    return do_fork(&regs, flags, 0, 0);
}
unsigned long do_execve(struct registers_in_stack *regs, char *name)
{
    unsigned long code_start_addr = 0x800000;
    unsigned long stack_start_addr = 0xa00000;
    unsigned long brk_start_addr = 0xc00000;
    unsigned long *tmp;
    unsigned long *virtual = NULL;
    struct Page *p = NULL;
    struct file *filp = NULL;
    unsigned long retval = 0;
    long pos = 0;

    regs->ds = KERNEL_DATA_SEGMENT;
    regs->es = KERNEL_DATA_SEGMENT;
    regs->ss = KERNEL_DATA_SEGMENT;
    regs->cs = KERNEL_CODE_SEGMENT;
    //	regs->rip = new_rip;
    //	regs->rsp = new_rsp;
    regs->r10 = 0x800000;
    regs->r11 = 0xa00000;
    regs->rax = 1;

    color_printk(YELLOW, BLACK, "do_execve task is running\n");

    if (current->flags & PF_VFORK)
    {
        current->mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
        memset(current->mm, 0, sizeof(struct mm_struct));

        current->mm->pgd = (pml4t_t *)V_TO_P(kmalloc(PAGE_4K_SIZE, 0));
        color_printk(RED, BLACK, "load_binary_file malloc new pgd:%#018lx\n", current->mm->pgd);
        memset((void *)P_TO_V(current->mm->pgd), 0, PAGE_4K_SIZE / 2);
        memcpy((void *)P_TO_V(init_task[smp_cpu_id()]->mm->pgd) + 256, (void *)P_TO_V(current->mm->pgd) + 256, PAGE_4K_SIZE / 2); // copy kernel space
    }

    tmp = (unsigned long *)P_TO_V((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
    if (*tmp == NULL)
    {
        virtual = (unsigned long *)kmalloc(PAGE_4K_SIZE, 0);
        memset(virtual, 0, PAGE_4K_SIZE);
        set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_USER_GDT));
    }

    tmp = (unsigned long *)P_TO_V((unsigned long *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
    if (*tmp == NULL)
    {
        virtual = (unsigned long *)kmalloc(PAGE_4K_SIZE, 0);
        memset(virtual, 0, PAGE_4K_SIZE);
        set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_USER_DIR));
    }

    tmp = (unsigned long *)P_TO_V((unsigned long *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff));
    if (*tmp == NULL)
    {
        p = alloc_pages(ZONE_NORMAL, 1, PAGE_PT_MAPED);
        set_pdt(tmp, make_pdt(p->p_address, PAGE_USER_PAGE));
    }
    asm volatile(
        "movq	%0,	%%cr3	\n"
        :
        : "r"(current->mm->pgd)
        : "memory");

    if (!(current->flags & PF_KTHREAD))
    {
        current->addr_limit = TASK_SIZE;
    }
    current->mm->start_code = code_start_addr;
    current->mm->end_code = 0;
    current->mm->start_data = 0;
    current->mm->end_data = 0;
    current->mm->start_rodata = 0;
    current->mm->end_rodata = 0;
    current->mm->start_bss = 0;
    current->mm->end_bss = 0;
    current->mm->start_brk = brk_start_addr;
    current->mm->end_brk = brk_start_addr;
    current->mm->start_stack = stack_start_addr;

    exit_file(current);

    current->flags &= ~PF_VFORK;

    filp = open_exec_file(name);

    if ((unsigned long)filp > -0x1000UL)
    {
        return (unsigned long)filp;
    }
    memset((void *)0x800000, 0, PAGE_2M_SIZE);
    retval = filp->f_ops->read(filp, (void *)0x800000, filp->dentry->dir_inode->file_size, &pos);

    return retval;
}

extern void system_call(void);

unsigned long init(unsigned long arg)
{
    disk_fat32_fs_init();
    color_printk(YELLOW, BLACK, "init task is running, arg code = %#018lx\n", arg);
    current->thread->rip = (unsigned long)ret_system_call;
    current->thread->rsp = (unsigned long)current + STACK_SIZE - sizeof(struct registers_in_stack);
    current->thread->fs = USER_DATA_SEGMENT;
    current->thread->gs = USER_DATA_SEGMENT;
    current->flags &= ~PF_KTHREAD;
    asm volatile(
        "movq %2, %%rsp\n"
        "pushq %1\n"
        "jmp do_execve\n"
        :
        : "D"(current->thread->rsp), "m"(current->thread->rip), "m"(current->thread->rsp), "S"("/init.bin")
        : "memory");
    return 1;
}
void task_init(void)
{
    spinlock_lock(&sequence_lock);
    unsigned long *tmp = NULL;
    unsigned long *vaddr = NULL;
    unsigned long *virtual = NULL;
    int i = 0;
    vaddr = (unsigned long *)P_TO_V((unsigned long)get_gdt() & (~0xfffUL));
    *vaddr = 0UL;
    for (i = 256; i < 512; i++)
    {
        tmp = vaddr + i;
        if (*tmp == 0)
        {
            virtual = (unsigned long *)kmalloc(PAGE_4K_SIZE, 0);
            memset(virtual, 0, PAGE_4K_SIZE);
            set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_KERNEL_GDT));
        }
    }
    init_mm.pgd = (pml4t_t *)get_gdt();
    init_mm.start_code = mem_structure.start_code;
    init_mm.end_code = mem_structure.end_code;
    init_mm.start_data = mem_structure.start_data;
    init_mm.end_data = mem_structure.end_data;
    init_mm.start_rodata = (unsigned long)&_rodata;
    init_mm.end_rodata = mem_structure.end_rodata;
    init_mm.start_brk = mem_structure.start_brk;
    init_mm.end_brk = current->addr_limit;
    init_mm.start_bss = (unsigned long)&_bss;
    init_mm.end_bss = (unsigned long)&_ebss;
    init_mm.start_stack = _stack_start_;
    wrmsr(0x174, KERNEL_CODE_SEGMENT);
    wrmsr(0x175, current->thread->rsp0);
    wrmsr(0x176, (unsigned long)system_call);
    init_tss[smp_cpu_id()].rsp0 = init_thread.rsp0;
    list_init(&current->list);
    kernel_thread(init, 10, CLONE_FS | CLONE_SIGNAL | CLONE_VM);
    spinlock_unlock(&sequence_lock);
    current->preempt_count = 0;
    current->state = TASK_RUNNING;
    current->cpu_id = 0;
    // p = container_of(list_next(&current->list), struct task_struct, list);
    // switch_to(current, p);
    return;
}
