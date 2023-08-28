#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
#include "cpu.h"
#include "task.h"

struct schedule
{
    long running_task_count;
    long CPU_exectask_jiffies;
    struct task_struct task_queue;
};
extern struct schedule task_schedule[NR_CPUS];

void schedule(void);
void schedule_init(void);
struct task_struct* get_next_task(void);
void insert_task_queue(struct task_struct* task);

#endif

