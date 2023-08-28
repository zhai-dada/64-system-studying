#include "atomic.h"

void atomic_add(atomic_t* atomic, long value)
{
    asm volatile
    (
        "lock addq %1, %0 \n"
        :"=m"(atomic->value)
        :"r"(value)
        :"memory"
    );
    return;
}

void atomic_sub(atomic_t* atomic, long value)
{
    asm volatile
    (
        "lock subq %1, %0 \n"
        :"=m"(atomic->value)
        :"r"(value)
        :"memory"
    );
    return;
}

void atomic_inc(atomic_t* atomic)
{
    asm volatile
    (
        "lock incq %0 \n"
        :"=m"(atomic->value)
        :"m"(atomic->value)
        :"memory"
    );
    return;
}

void atomic_dec(atomic_t* atomic)
{
    asm volatile
    (
        "lock decq %0 \n"
        :"=m"(atomic->value)
        :"m"(atomic->value)
        :"memory"
    );
    return;
}

void atomic_set_mask(atomic_t *atomic, long mask)
{
	asm volatile
    (	"lock orq %1, %0	\n"
		:"=m"(atomic->value)
		:"r"(mask)
		:"memory"
	);
    return;
}

void atomic_clear_mask(atomic_t *atomic, long mask)
{
	asm volatile
    (	"lock andq %1, %0	\n"
		:"=m"(atomic->value)
		:"r"(~(mask))
		:"memory"
	);
    return;
}