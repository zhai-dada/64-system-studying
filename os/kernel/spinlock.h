#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__
#include "preempt.h"

typedef struct
{
    volatile unsigned long lock; // 1 unlock 0 lock
} spinlock_t;

void spinlock_init(spinlock_t *lock);
void spinlock_lock(spinlock_t *lock);
void spinlock_unlock(spinlock_t *lock);
long spin_trylock(spinlock_t *lock);
#define local_irq_save(x)   \
    asm volatile\
    (\
        "pushfq\n"\
        "popq %0\n"\
        "cli\n"\
        :"=g"(x)\
        :\
        :"memory"\
    );
#define local_irq_restore(x)   \
    asm volatile\
    (\
        "pushq %0\n"\
        "popfq\n"\
        :\
        :"g"(x)\
        :"memory"\
    );
#define local_irq_disable() cli();
#define local_irq_enable() sti();

#endif