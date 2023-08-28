#ifndef __GATE_H__
#define __GATE_H__

struct GDT_Struct
{
    unsigned char GDT[8];
};
struct IDT_Struct
{
    unsigned char IDT[16];
};

extern struct GDT_Struct GDT_Table[];
extern struct IDT_Struct IDT_Table[];

#define set_gate(gate_addr, attr, ist, code_addr)                             \
    do                                                                        \
    {                                                                         \
        unsigned long d0, d1;                                                 \
        asm volatile(                                                         \
            "movw %%dx, %%ax	\n"                                              \
            "andq $0x7, %%rcx	\n"                                             \
            "addq %4, %%rcx     \n"                                           \
            "shlq $32, %%rcx	\n"                                              \
            "addq %%rcx, %%rax	\n"                                            \
            "xorq %%rcx, %%rcx	\n"                                            \
            "movl %%edx, %%ecx	\n"                                            \
            "shrq $16, %%rcx	\n"                                              \
            "shlq $48, %%rcx	\n"                                              \
            "addq %%rcx, %%rax	\n"                                            \
            "movq %%rax, %0	    \n"                                           \
            "shrq $32, %%rdx	\n"                                              \
            "movq %%rdx, %1     \n"                                           \
            : "=m"(*((unsigned long *)(gate_addr))),                          \
              "=m"(*(1 + (unsigned long *)(gate_addr))), "=&a"(d0), "=&d"(d1) \
            : "i"(attr << 8),                                                 \
              "3"((unsigned long *)(code_addr)), "2"(0x8 << 16), "c"(ist)     \
            : "memory");                                                      \
    } while (0)
#define load_TR(n)        \
    do                    \
    {                     \
        asm volatile(     \
            "ltr %%ax\n" \
            :             \
            : "a"(n << 3) \
            : "memory");  \
    } while (0)
void set_intr_gate(unsigned int n, unsigned char ist, void *addr)
{
    set_gate(IDT_Table + n, 0x8E, ist, addr);
    // P, DPL = 0, TYPE = E
    return;
}
void set_trap_gate(unsigned int n, unsigned char ist, void *addr)
{
    set_gate(IDT_Table + n, 0x8F, ist, addr);
    // P, DPL = 0, TYPE = F
    return;
}
void set_system_intr_gate(unsigned int n, unsigned char ist, void *addr)
{
    set_gate(IDT_Table + n, 0xEE, ist, addr);
    // P, DPL = 3, TYPE = E
    return;
}
void set_system_trap_gate(unsigned int n, unsigned char ist, void *addr)
{
    set_gate(IDT_Table + n, 0xEF, ist, addr);
    // P DPL = 3, TYPE = F
    return;
}
void set_tss64(unsigned int* tss_table, unsigned long rsp0, unsigned long rsp1, unsigned long rsp2, 
unsigned long ist1, unsigned long ist2, unsigned long ist3, unsigned long ist4, unsigned long ist5, unsigned long ist6, unsigned long ist7)
{
    *(unsigned long *)(tss_table + 1) = rsp0;
    *(unsigned long *)(tss_table + 3) = rsp1;
    *(unsigned long *)(tss_table + 5) = rsp2;

    *(unsigned long *)(tss_table + 9) = ist1;
    *(unsigned long *)(tss_table + 11) = ist2;
    *(unsigned long *)(tss_table + 13) = ist3;
    *(unsigned long *)(tss_table + 15) = ist4;
    *(unsigned long *)(tss_table + 17) = ist5;
    *(unsigned long *)(tss_table + 19) = ist6;
    *(unsigned long *)(tss_table + 21) = ist7;
    return;
}

void set_tss_descriptor(unsigned int n, void* addr)
{
    unsigned long limit = 103;
    *(unsigned long*)(GDT_Table + n) = (limit & 0xffff) | (((unsigned long)addr & 0xffff) << 16) | (((unsigned long)addr >> 16 & 0xff) << 32) | ((unsigned long)0x89 << 40) | ((limit >> 16 & 0xf) << 48) | (((unsigned long)addr >> 24 & 0xff) << 56);
    *(unsigned long*)(GDT_Table + n + 1) = ((unsigned long)addr >> 32 & 0xffffffff) | 0;
    return;
}
#endif