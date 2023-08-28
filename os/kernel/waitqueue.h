#ifndef __WAITQUEUE_H__
#define __WAITQUEUE_H__

#include "lib.h"

typedef struct
{
    struct List wait_list;
    struct task_struct *task;
} wait_queue_t;

void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *tsk);
void sleep_on(wait_queue_t *wait_queue_head);
void interruptible_sleep_on(wait_queue_t *wait_queue_head);
void wakeup(wait_queue_t *wait_queue_head, long state);

#endif