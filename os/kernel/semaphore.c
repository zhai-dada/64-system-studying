#include "semaphore.h"


void semaphore_init(semaphore_t *semaphore, unsigned long count)
{
    atomic_set(&semaphore->counter, count);
    wait_queue_init(&semaphore->wait, NULL);
    return;
}

void __down(semaphore_t *semaphore)
{
    wait_queue_t wait;
    wait_queue_init(&wait, current);
    current->state = TASK_UNINTERRUPTIBLE;
    list_add_before(&semaphore->wait.wait_list, &wait.wait_list);
    schedule();
    return;
}
void semaphore_down(semaphore_t *semaphore)
{
    if (atomic_read(&semaphore->counter) > 0)
    {
        atomic_dec(&semaphore->counter);
    }
    else
    {
        __down(semaphore);
    }
    return;
}
void __up(semaphore_t *semaphore)
{
    wait_queue_t *wait = container_of(list_next(&semaphore->wait.wait_list), wait_queue_t, wait_list);
    list_delete(&wait->wait_list);
    wait->task->state = TASK_RUNNING;
    insert_task_queue(wait->task);
    current->flags |= NEED_SHEDULE;
    return;
}
void semaphore_up(semaphore_t *semaphore)
{
    if (list_is_empty(&semaphore->wait.wait_list))
    {
        atomic_inc(&semaphore->counter);
    }
    else
    {
        __up(semaphore);
    }
    return;
}