#ifndef __ATOMIC_H__
#define __ATOMIC_H__

typedef struct 
{
    volatile long value;
}atomic_t;

void atomic_add(atomic_t* atomic, long value);
void atomic_sub(atomic_t* atomic, long value);
void atomic_inc(atomic_t* atomic);
void atomic_dec(atomic_t* atomic);
void atomic_set_mask(atomic_t *atomic, long mask);
void atomic_clear_mask(atomic_t *atomic, long mask);
#define atomic_read(atomic)	((atomic)->value)
#define atomic_set(atomic, val)	(((atomic)->value) = (val))
#endif