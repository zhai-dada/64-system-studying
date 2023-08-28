#ifndef __8259A_H__
#define __8259A_H__
#include "linkage.h"
#include "ptrace.h"
#include "interrupt.h"

void init_8259A(void);
void do_INT(struct registers_in_stack* regs, unsigned long nr);

#endif