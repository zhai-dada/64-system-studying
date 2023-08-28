#include "schedule.h"
#include "printk.h"
#include "memory.h"
#include "lib.h"
#include "interrupt.h"
#include "APIC.h"
#include "keyboard.h"
#include "spinlock.h"
#include "time.h"
#include "task.h"
#include "smp.h"

struct schedule task_schedule[NR_CPUS];
void schedule()
{
    cli();
    struct task_struct* task = NULL;
    current->flags &= ~NEED_SHEDULE;
    task = get_next_task();
    // color_printk(RED, BLACK, "#schedule:%d rip:%#018lx\n", jiffies, task->thread->rip);
    if(current->vruntime >= task->vruntime || current->state != TASK_RUNNING)
    {
        if(current->state == TASK_RUNNING)
        {
            insert_task_queue(current);
        }
        if(!task_schedule[smp_cpu_id()].CPU_exectask_jiffies)
        {
            switch(task->priority)
            {
                case 0:
                case 1:
                    task_schedule[smp_cpu_id()].CPU_exectask_jiffies = 4 / task_schedule[smp_cpu_id()].running_task_count;
                    break;
                case 2:
                default:
                    task_schedule[smp_cpu_id()].CPU_exectask_jiffies = 4 / task_schedule[smp_cpu_id()].running_task_count * 3;
                    break;

            }
        }
        switch_mm(current, task);
        switch_to(current, task);
    }
    else
    {
        insert_task_queue(task);
        if(!task_schedule[smp_cpu_id()].CPU_exectask_jiffies)
        {
            switch(task->priority)
            {
                case 0:
                case 1:
                    task_schedule[smp_cpu_id()].CPU_exectask_jiffies = 4 / task_schedule[smp_cpu_id()].running_task_count;
                    break;
                case 2:
                default:
                    task_schedule[smp_cpu_id()].CPU_exectask_jiffies = 4 / task_schedule[smp_cpu_id()].running_task_count * 3;
                    break;
            }
        }
    }
    sti();
    return;
}
void schedule_init()
{
    int i = 0;
    memset(&task_schedule[0], 0, sizeof(struct schedule) * NR_CPUS);
    for(i = 0; i < NR_CPUS; i++)
    {
        list_init(&task_schedule[i].task_queue.list);
        task_schedule[i].task_queue.vruntime = 0x7fffffffffffffff;
        task_schedule[i].running_task_count = 1;
        task_schedule[i].CPU_exectask_jiffies = 4;
    }
    return;
}
struct task_struct* get_next_task()
{
    struct task_struct* task = NULL;
    if(list_is_empty(&task_schedule[smp_cpu_id()].task_queue.list))
    {
        return &init_task_stack.task;
    }
    task = container_of(list_next(&task_schedule[smp_cpu_id()].task_queue.list), struct task_struct, list);
    list_delete(&task->list);
    task_schedule[smp_cpu_id()].running_task_count -= 1;
    return task;
}
void insert_task_queue(struct task_struct* task)
{
    struct task_struct* tmp = container_of(list_next(&task_schedule[smp_cpu_id()].task_queue.list), struct task_struct, list);
    if(task == &init_task_stack.task)
    {
        return;
    }
    if(list_is_empty(&task_schedule[smp_cpu_id()].task_queue.list))
    {
        ;
    }
    else
    {
        while(tmp->vruntime < task->vruntime)
        {
            tmp = container_of(list_next(&tmp->list), struct task_struct, list);
        }
    }
    list_add_before(&tmp->list, &task->list);
    task_schedule[smp_cpu_id()].running_task_count += 1;
    return;
}
