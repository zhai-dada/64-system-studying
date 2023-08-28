#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "linkage.h"
#include "ptrace.h"
#include "APIC.h"



extern void (*interrupt[24])(void);
extern void (*smp_interrupt[10])(void);
extern void do_INT(struct registers_in_stack* regs, unsigned long nr);

#define INT_NR 24
#define SMP_INT_NR  10

typedef struct
{
    void (*enable)(unsigned long irq);
    void (*disable)(unsigned long irq);
    unsigned long (*install)(unsigned long irq, void* arg);
    void (*uninstall)(unsigned long irq);
    void (*ack)(unsigned long irq);
}int_controler;
typedef struct
{
    int_controler* controler;
    char* int_name;
    unsigned long parameter;
    void (*handler)(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg);
    unsigned long flags;
}int_desc_t;


int_desc_t interrupt_desc[INT_NR] = {0};
int_desc_t SMP_IPI_desc[SMP_INT_NR] = {0};

int register_irq(unsigned long irq, void* arg, void (*handler)(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg), unsigned long parameter, int_controler* controler, char* int_name);
int unregister_irq(unsigned long irq);
int register_ipi(unsigned long irq, void* arg, void (*handler)(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg), unsigned long parameter, int_controler* controler, char* int_name);
int unregister_ipi(unsigned long irq);
#endif