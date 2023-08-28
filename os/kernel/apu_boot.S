#include "linkage.h"

.balign 0x1000
.text
.code16
ENTRY(APU_BOOT_START)
APU_BOOT_BASE = .
    cli
    wbinvd
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    movl $(APU_BOOT_TMP_STACK_END - APU_BOOT_BASE), %esp

    mov %cs, %ax
    movzx %ax, %esi
    shll $4, %esi
    leal (APU_CODE32 - APU_BOOT_BASE)(%esi), %eax
    movl %eax, APU_CODE32_VECTOR - APU_BOOT_BASE
    leal (APU_CODE64 - APU_BOOT_BASE)(%esi), %eax
    movl %eax, APU_CODE64_VECTOR - APU_BOOT_BASE
    leal (APU_TMP_GDT - APU_BOOT_BASE)(%esi), %eax
    movl %eax, (APU_TMP_GDT - APU_BOOT_BASE + 2)
    lidtl APU_TMP_IDT - APU_BOOT_BASE
    lgdtl APU_TMP_GDT - APU_BOOT_BASE
    smsw %ax
    bts $0, %ax
    lmsw %ax
    ljmpl *(APU_CODE32_VECTOR - APU_BOOT_BASE)
.code32
.balign 4
APU_CODE32:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs
    leal (APU_BOOT_TMP_STACK_END - APU_BOOT_BASE)(%esi), %eax
    movl %eax, %esp
    movl %cr4, %eax
    bts $5, %eax
    movl %eax, %cr4
    movl $0x101000, %eax
    movl %eax, %cr3
    movl $0xc0000080, %ecx
    rdmsr
    bts $8, %eax
    wrmsr
    movl %cr0, %eax
    bts $0, %eax
    bts $31, %eax
    movl %eax, %cr0
    ljmp *(APU_CODE64_VECTOR - APU_BOOT_BASE)(%esi)
.code64
.balign 4
APU_CODE64:
    movq $0x20, %rax
    movq %rax, %ds
    movq %rax, %es
    movq %rax, %fs
    movq %rax, %ss
    movq %rax, %gs

    movq $0x100000, %rax
    jmpq *%rax
    hlt

.balign 4
APU_TMP_IDT:
    .word 0
    .word 0, 0
APU_TMP_IDT_END:

.balign 4
APU_TMP_GDT:
    .short APU_TMP_GDT_END - APU_TMP_GDT - 1
    .long APU_TMP_GDT - APU_BOOT_BASE
    .short 0
    .quad 0x00cf9a000000ffff
    .quad 0x00cf92000000ffff
    .quad 0x0020980000000000
    .quad 0x0000920000000000
APU_TMP_GDT_END:

.balign 4
APU_CODE32_VECTOR:
    .long APU_CODE32 - APU_BOOT_BASE
    .word 0x08, 0

.balign 4
APU_CODE64_VECTOR:
    .long APU_CODE64 - APU_BOOT_BASE
    .word 0x18, 0

.balign 4
APU_BOOT_TMP_STACK_START:
    .org 0x400
APU_BOOT_TMP_STACK_END:

ENTRY(APU_BOOT_END)