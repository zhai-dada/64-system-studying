#include "waitqueue.h"
#include "task.h"
#include "schedule.h"

void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *task)
{
    list_init(&wait_queue->wait_list);
    wait_queue->task = task;
    return;
}
void sleep_on(wait_queue_t *wait_queue_head)
{
    wait_queue_t wait;
    wait_queue_init(&wait, current);
    current->state = TASK_UNINTERRUPTIBLE;
    list_add_before(&wait_queue_head->wait_list, &wait.wait_list);
    schedule();
}

void interruptible_sleep_on(wait_queue_t *wait_queue_head)
{
    wait_queue_t wait;
    wait_queue_init(&wait, current);
    current->state = TASK_INTERRUPTIBLE;
    list_add_before(&wait_queue_head->wait_list, &wait.wait_list);
    schedule();
    return;
}

void wakeup(wait_queue_t *wait_queue_head, long state)
{
    wait_queue_t *wait = NULL;

    if (list_is_empty(&wait_queue_head->wait_list))
    {
        return;
    }
    wait = container_of(list_next(&wait_queue_head->wait_list), wait_queue_t, wait_list);

    if (wait->task->state & state)
    {
        list_delete(&wait->wait_list);
        wakeup_process(wait->task);
    }
    // schedule();
    return;
}