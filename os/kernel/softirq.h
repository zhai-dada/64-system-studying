#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__
#include "lib.h"
#include "memory.h"
#include "APIC.h"
#include "interrupt.h"
#include "keyboard.h"
#include "HPET.h"
#include "printk.h"

#define TIMER_S_IRQ     (1 << 0)

unsigned long volatile softirq_status = 0;
unsigned long volatile jiffies = 0;
struct softirq
{
    void (*action)(void* data);
    void* data;
};
struct softirq softirq_vector[64] = {0};

void set_softirq_status(unsigned long status);
unsigned long get_softirq_status(void);
void register_softirq(int nr, void (*action)(void* data), void* data);
void unregister_softirq(int nr);
void softirq_init(void);
void do_softirq(void);

#endif