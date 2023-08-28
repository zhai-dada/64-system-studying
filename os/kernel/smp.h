#ifndef __SMP_H__
#define __SMP_H__

#include "lib.h"
#include "spinlock.h"

extern spinlock_t smp_lock;

extern unsigned char APU_BOOT_START[];
extern unsigned char APU_BOOT_END[];
#define smp_cpu_id() (current->cpu_id)

void smp_init(void);
void start_smp(void);
#endif