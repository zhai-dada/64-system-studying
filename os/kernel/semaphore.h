#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "atomic.h"
#include "lib.h"
#include "task.h"
#include "schedule.h"
#include "waitqueue.h"

typedef struct
{
    atomic_t counter;
    wait_queue_t wait;
}semaphore_t;


void semaphore_init(semaphore_t* semaphore, unsigned long count);
void semaphore_down(semaphore_t* semaphore);
void semaphore_up(semaphore_t* semaphore);
#endif