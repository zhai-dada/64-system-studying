#ifndef __APIC_H__
#define __APIC_H__
#include "linkage.h"
#include "ptrace.h"
#include "interrupt.h"
#include "lib.h"
#include "spinlock.h"

//deliver mode
#define IOAPIC_FIXED    0
#define IOAPIC_LOWESTPRIORITY   1
#define IOAPIC_SMI  2
#define IOAPIC_NMI  4
#define IOAPIC_INIT 5
#define IOAPIC_EXTINT   7
#define IOAPIC_ICR_START_UP  6
//destination mode
#define IOAPIC_DEST_MODE_PHYSICAL   0
#define IOAPIC_DEST_MODE_LOGICAL    1
//deliver status
#define IOAPIC_DELI_STATUS_IDLE 0
#define IOAPIC_DELI_STATUS_SEND 1
//polarity
#define IOAPIC_POLARITY_HIGH    0
#define IOAPIC_POLARITY_LOW     1
//irr
#define IOAPIC_IRR_RESET    0
#define IOAPIC_IRR_ACCEPT   1
//trigger
#define IOAPIC_TRIGGER_EDGE     0
#define IOAPIC_TRIGGER_LEVEL    1
//mask
#define IOAPIC_MASK_MASKED  1
#define IOAPIC_MASK_UNMASK  0
//shorthand
#define ICR_NO_SHORTHAND		    0
#define ICR_SELF		        	1
#define ICR_ALL_INCLUDE_SELF		2
#define ICR_ALL_EXCLUDE_SELF		3
//level
#define ICR_LEVEL_DE_ASSERT		0
#define ICR_LEVLE_ASSERT		1

#define EOI()   \
    asm volatile    \
    (               \
        "movq $0x00, %%rax\n"\
        "movq $0x00, %%rdx\n"\
        "movq $0x80b, %%rcx\n"\
        "wrmsr\n"\
        :\
        :\
        :"memory"\
    );
struct IOAPIC_map
{
    unsigned int physical_address;
    unsigned char* virtual_index_address;
    unsigned int* virtual_data_address;
    unsigned int* virtual_eoi_address;
}ioapic_map;

struct IOAPIC_RET_ENTRY
{
	unsigned int 
    vector_num	    :8,	//0~7
	deliver_mode	:3,	//8~10
	dest_mode	    :1,	//11
	deliver_status	:1,	//12
	polarity        :1,	//13
	irr	            :1,	//14
	trigger	        :1,	//15
	mask	        :1,	//16
	reserved        :15;	//17~31
	union
    {
        struct 
        {
			unsigned int
            reserved1	:24,	//32~55
			phy_dest	:4,	//56~59
			reserved2	:4;	//60~63	
        }physical;
		struct 
        {
			unsigned int
            reserved1	    :24,	//32~55
			logical_dest	:8;	//56~63
		}logical;
	}destination;
}PACKED;

struct INT_CMD_REG
{
	unsigned int
        vector_num  	:8,	//0~7
		deliver_mode	:3,	//8~10
		dest_mode	    :1,	//11
		deliver_status	:1,	//12
		res_1	        :1,	//13
		level	        :1,	//14
		trigger	        :1,	//15
    	res_2	        :2,	//16~17
		dest_shorthand	:2,	//18~19
		res_3       	:12;	//20~31
	union 
    {
		struct
        {
			unsigned int
            res_4	    :24,	//32~55
			dest_field	:8;	//56~63		
		}apic_destination;
			
		unsigned int
            x2apic_destination;	//32~63
	}destination;
		
}PACKED;

void local_APIC_init(void);
void APIC_IOAPIC_init(void);
void do_INT(struct registers_in_stack* regs, unsigned long nr);
void IOAPIC_page_table_remap(void);
unsigned long ioapic_rte_read(unsigned char index);
void ioapic_rte_write(unsigned char index, unsigned long value);
void IOAPIC_init(void);
void getIRR(void);
void getISR(void);
void IOAPIC_enable(unsigned long irq);
void IOAPIC_disable(unsigned long irq);
void IOAPIC_uninstall(unsigned long irq);
void IOAPIC_level_ack(unsigned long irq);
void IOAPIC_edge_ack(unsigned long irq);
unsigned long IOAPIC_install(unsigned long irq,void * arg);

extern spinlock_t sequence_lock;

#endif
