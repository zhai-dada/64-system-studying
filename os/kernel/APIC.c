#include "printk.h"
#include "memory.h"
#include "APIC.h"
#include "cpu.h"
#include "lib.h"
#include "gate.h"
#include "ptrace.h"
#include "linkage.h"
#include "interrupt.h"
#include "smp.h"
#include "task.h"

void local_APIC_init()
{
    unsigned long x = 0;
    unsigned long y = 0;
    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;
    cpuid(1, 0, &a, &b, &c, &d);
    color_printk(WHITE, BLACK, "CPUID\t01, eax:%#010x, ebx:%#010x, ecx:%#010x, edx:%#010x\n", a, b, c, d);
    if((1 << 9) & d)
    {
        color_printk(WHITE, BLACK, "support APIC&xAPIC\t");
    }
    else
    {
        color_printk(WHITE, BLACK, "No support APIC&xAPIC\t");
    }
    if((1 << 21) & c)
    {
        color_printk(WHITE, BLACK, "support x2APIC\n");
    }
    else
    {
        color_printk(WHITE, BLACK, "No support x2APIC\n");
    }
    asm volatile
    (
        "movq $0x1b, %%rcx\n"
        "rdmsr\n"
        "bts $10, %%rax\n"
        "bts $11, %%rax\n"
        "wrmsr\n"
        "movq $0x1b, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(WHITE, BLACK, "eax:%010x, edx:%#010x\t", x, y);
    if(x & 0x0c00)
    {
        color_printk(WHITE, BLACK, "xAPIC & x2APIC enabled\n");
    }
    asm volatile
    (
        "movq $0x80f, %%rcx\n"
        "rdmsr\n"
        "bts $8, %%rax\n"
        #ifndef BOCHS
        "bts $12, %%rax\n"
        #endif
        "wrmsr\n"
        "movq $0x80f, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(WHITE, BLACK, "eax:%#010x, edx:%#010x\t", x, y);
    if(x & 0x100)
    {
        color_printk(WHITE, BLACK, "SVR[8] enabled\t");
    }
    if(x & 0x1000)
    {
        color_printk(WHITE, BLACK, "SVR[12] enabled");
    }
    color_printk(WHITE, BLACK, "\n");
    asm volatile
    (
        "movq $0x802, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(WHITE, BLACK, "eax:%#010x, edx:%#010x\tx2APIC ID:%#010x\n", x, y, x);
    asm volatile
    (
        "movq $0x803, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(WHITE, BLACK, "Local APIC version:%#010x, Max LVT Entry num:%#010x, SVR(EOI):%#04x\t", x & 0xff, (x >> 16) & 0xff + 1, x >> 24 & 0x1);
    if((x & 0xff) < 0x10)
    {
        color_printk(WHITE, BLACK, "82489DX discrete APIC\n");
    }
    else if(((x & 0xff) >= 0x10) && ((x & 0xff) <= 0x15))
    {
        color_printk(WHITE, BLACK, "Integrated APIC\n");
    }
    asm volatile
    (
        "movq $0x82f, %%rcx\n"
        "wrmsr\n"
        "movq $0x832, %%rcx\n"
        "wrmsr\n"
        "movq $0x833, %%rcx\n"
        "wrmsr\n"
        "movq $0x834, %%rcx\n"
        "wrmsr\n"
        "movq $0x835, %%rcx\n"
        "wrmsr\n"
        "movq $0x836, %%rcx\n"
        "wrmsr\n"
        "movq $0x837, %%rcx\n"
        "wrmsr\n"
        :
        :"a"(0x10000), "d"(0x00)
        :"memory"
    );
    color_printk(GREEN, BLACK, "Mask LVT\n");
    asm volatile
    (
        "movq $0x808, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(GREEN, BLACK, "Set LVT TPR:%#010x\t", x);
    asm volatile
    (
        "movq $0x80a, %%rcx\n"
        "rdmsr\n"
        :"=a"(x), "=d"(y)
        :
        :"memory"
    );
    color_printk(GREEN, BLACK, "Set LVT PPR:%#010x\n", x);
    //write 0 to ESR
    asm volatile
    (
        "movq $0x828, %%rcx\n"
        "wrmsr\n"
        :
        :"a"(0), "d"(0)
        :"memory"
    );
    color_printk(GREEN, BLACK, "Write 0 to ESR\n");
    return;
}
void APIC_IOAPIC_init()
{
    int i = 0;
    unsigned int x = 0;
    unsigned short* p = NULL;
    color_printk(GREEN, BLACK, "Mask 8259A\n");
    io_out8(0x21, 0xff);
    io_out8(0xa1, 0xff);
    IOAPIC_page_table_remap();
    for(i = 0x20; i <= 0x37; i++)
    {
        set_intr_gate(i, 0, interrupt[i - 0x20]);
    }
    //IMCR
    io_out8(0x22, 0x70);
    io_out8(0x23, 0x01);
    local_APIC_init();
    IOAPIC_init();
    // getIRR();
    // getISR();
    io_mfence();
    // getIRR();
    // getISR();
    memset(interrupt_desc, 0, sizeof(int_desc_t) * INT_NR);
    io_mfence();
    return;
}

void IOAPIC_page_table_remap()
{
    unsigned long* tmp = NULL;
    unsigned long* virtual = NULL;
    unsigned char* IOAPIC_addr = (unsigned char*)P_TO_V(0x0fec00000);
    ioapic_map.physical_address = 0xfec00000;
    ioapic_map.virtual_index_address = IOAPIC_addr;
    ioapic_map.virtual_data_address = (unsigned int*)(IOAPIC_addr + 0x10);
    ioapic_map.virtual_eoi_address = (unsigned int*)(IOAPIC_addr + 0x40);
    GLOBAL_CR3 = get_gdt();
	tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)GLOBAL_CR3 & (~0xfffUL))) + ((unsigned long)((unsigned long)IOAPIC_addr >> PAGE_GDT_SHIFT) & (0x1ff)) * 8);
    if(*tmp == 0)
    {
        color_printk(RED, BLACK, "ioapic,index:%#018lx\n", ((unsigned long)(0xffff8000fec00000 >> PAGE_GDT_SHIFT) ));
        virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
        memset(virtual, 0, PAGE_4K_SIZE);
        set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_KERNEL_GDT));
    }
    color_printk(YELLOW, BLACK, "1:%#018lx\t%#018lx\n", (unsigned long)tmp, (unsigned long)(*tmp));
	tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + (((unsigned long)(IOAPIC_addr) >> PAGE_1G_SHIFT) & 0x1ff) * 8);
    if(*tmp == 0)
    {
        virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
        memset(virtual, 0, PAGE_4K_SIZE);
        set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_KERNEL_DIR));
    }
    color_printk(YELLOW, BLACK, "2:%#018lx\t%#018lx\n", (unsigned long)tmp, (unsigned long)(*tmp));
	tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + (((unsigned long)(IOAPIC_addr) >> PAGE_2M_SHIFT) & 0x1ff) * 8);
    set_pdt(tmp, make_pdt(ioapic_map.physical_address, PAGE_KERNEL_PAGE | PAGE_PWT | PAGE_PCD));
    color_printk(YELLOW, BLACK, "3:%#018lx\t%#018lx\n", (unsigned long)tmp, (unsigned long)(*tmp));
    color_printk(GREEN, BLACK, "ioapic_map.physical_address:%#018lx\n", ioapic_map.physical_address);
    color_printk(GREEN, BLACK, "ioapic_map.virtual_address:%#018lx\n", ioapic_map.virtual_index_address);
    flush_tlb();
    return;
}
unsigned long ioapic_rte_read(unsigned char index)
{
    unsigned long ret = 0;
    *ioapic_map.virtual_index_address = index + 1;
    io_mfence();
    ret = *ioapic_map.virtual_data_address;
    ret = ret << 32;
    io_mfence();
    *ioapic_map.virtual_index_address = index;
    io_mfence();
    ret = ret | *ioapic_map.virtual_data_address;
    io_mfence();
    return ret;
}
void ioapic_rte_write(unsigned char index, unsigned long value)
{
    *ioapic_map.virtual_index_address = index;
    io_mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    value = value >> 32;
    io_mfence();
    *ioapic_map.virtual_index_address = index + 1;
    io_mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    io_mfence();
    return;
}
void IOAPIC_init()
{
    unsigned long test = 0;
    int i = 0;
    *ioapic_map.virtual_index_address = 0x00;
    io_mfence();
    *ioapic_map.virtual_data_address = 0x0f000000;
    io_mfence();
    color_printk(GREEN,BLACK,"Get IOAPIC ID Register:%#010x,ID:%#010x\n", *ioapic_map.virtual_data_address, *ioapic_map.virtual_data_address >> 24 & 0xf);
	io_mfence();
	*ioapic_map.virtual_index_address = 0x01;
	io_mfence();
	color_printk(GREEN,BLACK,"Get IOAPIC Version Register:%#010x,RTE count:%#08d\n", *ioapic_map.virtual_data_address, ((*ioapic_map.virtual_data_address >> 16) & 0xff) + 1);
    #ifdef BOCHS
    for(i = 0x10; i < 0x40; i = i + 2)
	{
        ioapic_rte_write(i, 0x10020 + ((i - 0x10) >> 1));
    }
    #else
    for(i = 0x10; i < 0x100; i = i + 2)
	{
        ioapic_rte_write(i, 0x10020 + ((i - 0x10) >> 1));
    }
    #endif
	color_printk(GREEN,BLACK,"I/O APIC Redirection Table Entries Set Finished.\n");	
    return;
}
void getIRR()
{
    int a = 0xffff, b = 0xffff;
    asm volatile
    (
        "movq $0x820, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:0-31bit:%#010x", a);
    asm volatile
    (
        "movq $0x821, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:32-63bit:%#010x", a);
    asm volatile
    (
        "movq $0x822, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:64-95bit:%#010x", a);
    asm volatile
    (
        "movq $0x823, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:96-127bit:%#010x", a);
    asm volatile
    (
        "movq $0x824, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:128-159bit:%#010x", a);
    asm volatile
    (
        "movq $0x825, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:160-191bit:%#010x", a);
    asm volatile
    (
        "movq $0x826, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:192-223bit:%#010x", a);
    asm volatile
    (
        "movq $0x827, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "IRR:224-255bit:%#010x\n", a);
    return;
}
void getISR()
{
    int a = 0, b = 0;
    asm volatile
    (
        "movq $0x810, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:0-31bit:%#010x", a);
    asm volatile
    (
        "movq $0x811, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:32-63bit:%#010x", a);
    asm volatile
    (
        "movq $0x812, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:64-95bit:%#010x", a);
    asm volatile
    (
        "movq $0x813, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:96-127bit:%#010x", a);
    asm volatile
    (
        "movq $0x814, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:128-159bit:%#010x", a);
    asm volatile
    (
        "movq $0x815, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:160-191bit:%#010x", a);
    asm volatile
    (
        "movq $0x816, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:192-223bit:%#010x", a);
    asm volatile
    (
        "movq $0x817, %%rcx\n"
        "rdmsr\n"
        :"=a"(a), "=d"(b)
        :
        :"memory"
    );
    color_printk(YELLOW, BLACK, "ISR:224-255bit:%#010x\n", a);
    return;
}

void IOAPIC_enable(unsigned long irq)
{
	unsigned long value = 0;
	value = ioapic_rte_read((irq - 0x20) * 2 + 0x10);
	value = value & (~0x10000UL); 
	ioapic_rte_write((irq - 0x20) * 2 + 0x10, value);
    return;
}
void IOAPIC_disable(unsigned long irq)
{
	unsigned long value = 0;
	value = ioapic_rte_read((irq - 0x20) * 2 + 0x10);
	value = value | (0x10000UL); 
	ioapic_rte_write((irq - 0x20) * 2 + 0x10, value);
    return;
}
void IOAPIC_uninstall(unsigned long irq)
{
	ioapic_rte_write((irq - 32) * 2 + 0x10, 0x10000UL);
    return;
}
unsigned long IOAPIC_install(unsigned long irq,void * arg)
{
	struct IOAPIC_RET_entry *entry = (struct IOAPIC_RET_entry *)arg;
	ioapic_rte_write((irq - 32) * 2 + 0x10, *(unsigned long *)entry);
	return 1;
}
void IOAPIC_level_ack(unsigned long irq)
{
    EOI();				
	*ioapic_map.virtual_eoi_address = irq;
    return;
}
void IOAPIC_edge_ack(unsigned long irq)
{
    EOI();
    return;
}
void do_INT(struct registers_in_stack* regs, unsigned long nr)
{
    switch(nr & 0x80)
    {
        case 0x00:
        {
            int_desc_t* irq = &interrupt_desc[nr - 0x20];
            if(irq->handler != NULL)
            {
                irq->handler(nr, irq->parameter, regs);
            }
            if(irq->controler != NULL && irq->controler->ack != NULL)
            {
                irq->controler->ack(nr);
            }
            break;
        }
        case 0x80:
        {
            IOAPIC_edge_ack(nr);
            int_desc_t* irq = &SMP_IPI_desc[nr - 200];
            if(irq->handler != NULL)
            {
                irq->handler(nr, irq->parameter, regs);
            }
            break;
        }
        default:
        {
            color_printk(RED, BLACK, "INT:%d\n", nr);
            break;
        }
    }
    // spinlock_unlock(&sequence_lock);
    return;
}