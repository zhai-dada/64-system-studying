#include "spinlock.h"
#include "task.h"
void spinlock_init(spinlock_t* lock)
{
    lock->lock = 1;
    return;
}
void spinlock_lock(spinlock_t* lock)
{
    preempt_disable();
    asm volatile
    (
        "1:             \n"
        "lock decq %0   \n"
        "jns 3f         \n"
        "2:             \n"
        "pause          \n"
        "cmpq $0, %0    \n"
        "jle 2b         \n"
        "jmp 1b         \n"
        "3:             \n"
        :"=m"(lock->lock)
        :
        :"memory"
    );
    io_mfence();
    return;
}
void spinlock_unlock(spinlock_t* lock)
{
    asm volatile
    (
        "movq $1, %0    \n"
        :"=m"(lock->lock)
        :
        :"memory"
    );
    io_mfence();
    preempt_enable();
    return;
}
long spin_trylock(spinlock_t * lock)
{
	unsigned long tmp_value = 0;
	preempt_disable();
	asm volatile
    (	
        "xchgq	%0,	%1	\n\t"
		:"=q"(tmp_value),"=m"(lock->lock)
		:"0"(0)
		:"memory"
	);
	if(!tmp_value)
	{
        preempt_enable();
    }
	return tmp_value;
}
