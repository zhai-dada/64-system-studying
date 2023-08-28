#include "syscall.h"

#define SYSFUNC_DEF(name) _SYSFUNC_DEF_(name, name##_nr)
#define _SYSFUNC_DEF_(name, nr) __SYSFUNC_DEF__(name, nr)
#define __SYSFUNC_DEF__(name, nr)                           \
    asm                 \
    (                                                \
        ".global "#name"	\n"                            \
        ".type	"#name",	@function \n"#name": \n" \
        "movq	$"#nr",	%rax	\n"                        \
        "jmp	SYSCALL	\n"             \
    );

SYSFUNC_DEF(putstring)

SYSFUNC_DEF(open)
SYSFUNC_DEF(close)
SYSFUNC_DEF(read)
SYSFUNC_DEF(write)
SYSFUNC_DEF(lseek)

SYSFUNC_DEF(fork)
SYSFUNC_DEF(vfork)
SYSFUNC_DEF(brk)
SYSFUNC_DEF(device_openkeyboard)
asm
(
    "SYSCALL:	\n"
    "pushq	%r10	\n"
    "pushq	%r11	\n"
    "leaq	sysexit_return_address(%rip),	%r10	\n"
    "movq	%rsp,	%r11		\n"
    "sysenter			\n"
    "sysexit_return_address:	\n"
    "xchgq	%rdx,	%r10	\n"
    "xchgq	%rcx,	%r11	\n"
    "popq	%r11	\n"
    "popq	%r10	\n"
    "cmpq	$-0x1000,	%rax	\n"
    "jb	SYSCALL_RET	\n"
    "movq	%rax,	errno(%rip)	\n"
    "orq	$-1,	%rax	\n"
    "SYSCALL_RET:	\n"
    "retq	\n"
);