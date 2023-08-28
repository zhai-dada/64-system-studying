#include "interrupt.h"
#include "linkage.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"
#include "lib.h"

#define SAVE_ALL                    \
    "cld                        \n" \
    "pushq %rax                 \n" \
    "pushq %rax                 \n" \
    "movq %es, %rax             \n" \
    "pushq %rax                 \n" \
    "movq %ds, %rax             \n" \
    "pushq %rax                 \n" \
    "xorq %rax, %rax            \n" \
    "pushq %rbp                 \n" \
    "pushq %rdi                 \n" \
    "pushq %rsi                 \n" \
    "pushq %rdx                 \n" \
    "pushq %rcx                 \n" \
    "pushq %rbx                 \n" \
    "pushq %r8                  \n" \
    "pushq %r9                  \n" \
    "pushq %r10                 \n" \
    "pushq %r11                 \n" \
    "pushq %r12                 \n" \
    "pushq %r13                 \n" \
    "pushq %r14                 \n" \
    "pushq %r15                 \n" \
    "movq $0x10, %rdx           \n" \
    "movq %rdx, %ds             \n" \
    "movq %rdx, %es             \n"
#define INT_NAME2(nr) nr##_interrupt(void)
#define INT_NAME(nr) INT_NAME2(INT##nr)

#define Build_INT(nr)                                   \
void INT_NAME(nr);                                      \
    asm(                                                \
        SYMBOL_NAME_STR(INT)#nr"_interrupt:     \n"     \
        "pushq $0x00                            \n"     \
        SAVE_ALL                                        \
        "movq %rsp, %rdi                        \n"     \
        "leaq ret_from_intr(%rip), %rax         \n"     \
        "pushq %rax                             \n"     \
        "movq $"#nr", %rsi                      \n"     \
        "jmp do_INT                             \n");


Build_INT(0x20)
Build_INT(0x21)
Build_INT(0x22)
Build_INT(0x23)
Build_INT(0x24)
Build_INT(0x25)
Build_INT(0x26)
Build_INT(0x27)
Build_INT(0x28)
Build_INT(0x29)
Build_INT(0x2a)
Build_INT(0x2b)
Build_INT(0x2c)
Build_INT(0x2d)
Build_INT(0x2e)
Build_INT(0x2f)
Build_INT(0x30)
Build_INT(0x31)
Build_INT(0x32)
Build_INT(0x33)
Build_INT(0x34)
Build_INT(0x35)
Build_INT(0x36)
Build_INT(0x37)

Build_INT(0xc8)
Build_INT(0xc9)
Build_INT(0xca)
Build_INT(0xcb)
Build_INT(0xcc)
Build_INT(0xcd)
Build_INT(0xce)
Build_INT(0xcf)
Build_INT(0xd0)
Build_INT(0xd1)



void (* interrupt[24])(void) = 
{
    INT0x20_interrupt,
    INT0x21_interrupt,
    INT0x22_interrupt,
    INT0x23_interrupt,
    INT0x24_interrupt,
    INT0x25_interrupt,
    INT0x26_interrupt,
    INT0x27_interrupt,
    INT0x28_interrupt,
    INT0x29_interrupt,
    INT0x2a_interrupt,
    INT0x2b_interrupt,
    INT0x2c_interrupt,
    INT0x2d_interrupt,
    INT0x2e_interrupt,
    INT0x2f_interrupt,
    INT0x30_interrupt,
    INT0x31_interrupt,
    INT0x32_interrupt,
    INT0x33_interrupt,
    INT0x34_interrupt,
    INT0x35_interrupt,
    INT0x36_interrupt,
    INT0x37_interrupt
};
void (*smp_interrupt[10])(void) = 
{
    INT0xc8_interrupt,
    INT0xc9_interrupt,
    INT0xca_interrupt,
    INT0xcb_interrupt,
    INT0xcc_interrupt,
    INT0xcd_interrupt,
    INT0xce_interrupt,
    INT0xcf_interrupt,
    INT0xd0_interrupt,
    INT0xd1_interrupt
};

int register_irq(unsigned long irq, void* arg, void (*handler)(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg), unsigned long parameter, int_controler* controler, char* int_name)
{
    int_desc_t* p = &interrupt_desc[irq - 0x20];
    p->controler = controler;
    p->int_name = int_name;
    p->parameter = parameter;
    p->flags = 0;
    p->handler = handler;
    p->controler->install(irq, arg);
    p->controler->enable(irq);
    return 1;
}
int unregister_irq(unsigned long irq)
{
    int_desc_t* p = &interrupt_desc[irq - 0x20];
    p->controler->disable(irq);
    p->controler->uninstall(irq);
    p->handler = NULL;
    p->flags = 0;
    p->parameter = 0;
    p->controler = NULL;
    p->int_name = NULL;
    return 1;
}
int register_ipi(unsigned long irq, void* arg, void (*handler)(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg), unsigned long parameter, int_controler* controler, char* int_name)
{
    int_desc_t* p = &SMP_IPI_desc[irq - 200];
    p->controler = NULL;
    p->int_name = int_name;
    p->parameter = parameter;
    p->flags = 0;
    p->handler = handler;
    return 1;
}
int unregister_ipi(unsigned long irq)
{
    int_desc_t* p = &SMP_IPI_desc[irq - 200];
    p->controler = NULL;
    p->int_name = NULL;
    p->parameter = NULL;
    p->flags = 0;
    p->handler = NULL;
    return 1;
}