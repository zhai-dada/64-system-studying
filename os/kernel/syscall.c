
#define SYS_CALL(nr, sym) extern unsigned long sym(void);
SYS_CALL(0, no_system_call)
#include "syscall.h"
#undef SYS_CALL

#define SYS_CALL(nr, sym) [nr] = sym,
#define SYS_CALL_NUM    128
typedef unsigned long (*system_call_t)(void);

system_call_t system_call_table[SYS_CALL_NUM] = 
{
    [0 ... SYS_CALL_NUM - 1] = no_system_call,
    #include "syscall.h"
};