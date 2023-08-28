#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "printk.h"
#include "lib.h"
#define TEST color_printk(WHITE, BLACK, "H\n");

extern unsigned long* GLOBAL_CR3;

#define COUNT_PER_PAGE  512

#define PAGE_OFFSET         ((unsigned long)0xffff800000000000)
#define PAGE_GDT_SHIFT  39
#define PAGE_1G_SHIFT   30
#define PAGE_2M_SHIFT	21
#define PAGE_4K_SHIFT	12

#define PAGE_2M_SIZE        (1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE        (1UL << PAGE_4K_SHIFT)

#define PAGE_2M_MASK        (~(PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK        (~(PAGE_4K_SIZE - 1))

#define PAGE_2M_ALIGN(addr) (((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr) (((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)

#define V_TO_P(addr)        ((unsigned long)(addr) - PAGE_OFFSET)
#define P_TO_V(addr)        ((unsigned long)(addr) + PAGE_OFFSET)



typedef struct
{
    unsigned long pml4t;
} pml4t_t;
#define make_pml4t(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pml4t(pml4t_ptr, pml4t_val) (*(pml4t_ptr) = (pml4t_val))

typedef struct
{
    unsigned long pdpt;
} pdpt_t;
#define make_pdpt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdpt(pdpt_ptr, pdpt_val) (*(pdpt_ptr) = (pdpt_val))

typedef struct
{
    unsigned long pdt;
} pdt_t;
#define make_pdt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdt(pdt_ptr, pdt_val) (*(pdt_ptr) = (pdt_val))

typedef struct
{
    unsigned long pt;
} pt_t;
#define make_pt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pt(pdpt_ptr, pdpt_val) (*(pt_ptr) = (pt_val))

#define PAGE_XD		(1UL << 63)
#define	PAGE_PAT	(1UL << 12)
#define	PAGE_GLOBAL	(1UL << 8)
#define	PAGE_PS		(1UL << 7)
#define	PAGE_DIRTY	(1UL << 6)
#define	PAGE_ACCESSED	(1UL << 5)
#define PAGE_PCD	(1UL << 4)
#define PAGE_PWT	(1UL << 3)
#define	PAGE_U_S	(1UL << 2)
#define	PAGE_R_W	(1UL << 1)
#define	PAGE_PRESENT	(1UL << 0)

#define PAGE_KERNEL_GDT		(PAGE_R_W | PAGE_PRESENT)
#define PAGE_KERNEL_DIR		(PAGE_R_W | PAGE_PRESENT)
#define	PAGE_KERNEL_PAGE	(PAGE_PS  | PAGE_R_W | PAGE_PRESENT)
#define PAGE_USER_GDT		(PAGE_U_S | PAGE_R_W | PAGE_PRESENT)
#define PAGE_USER_DIR		(PAGE_U_S | PAGE_R_W | PAGE_PRESENT)
#define	PAGE_USER_PAGE		(PAGE_PS  | PAGE_U_S | PAGE_R_W | PAGE_PRESENT)

struct E820_Format
{
    unsigned long address;
    unsigned long length;
    unsigned int type;
}PACKED;
struct Global_Memory_Descriptor
{
    struct E820_Format E820[32];
    unsigned long E820_length;
    unsigned long* bits_map;
    unsigned long bits_size;
    unsigned long bits_length;
    struct Page* pages_struct;
    unsigned long pages_size;
    unsigned long pages_length;
    struct Zone* zones_struct;
    unsigned long zones_size;
    unsigned long zones_length;
    unsigned long start_code, end_code;
    unsigned long start_data, end_data;
    unsigned long end_rodata;
    unsigned long start_brk;
    unsigned long end_of_struct;
};
struct Page
{
    struct Zone* zone_struct;
    unsigned long p_address;
    unsigned long attribute;
    unsigned long reference_count;
    unsigned long age;
};
extern int ZONE_DMA_INDEX;
extern int ZONE_NORMAL_INDEX;
extern int ZONE_UNMAPED_INDEX;
#define MAX_ZONES 10
struct Zone
{
    struct Page* pages_group;
    unsigned long pages_length;
    unsigned long zone_start_address;
    unsigned long zone_end_address;
    unsigned long zone_length;
    unsigned long attritube;
    struct Global_Memory_Descriptor* GMD_struct;
    unsigned long page_using_count;
    unsigned long page_free_count;
    unsigned long total_pages_link;
};
extern struct Global_Memory_Descriptor mem_structure;
//zone select
#define ZONE_DMA            (1 << 0)
#define ZONE_NORMAL         (1 << 1)
#define ZONE_UNMAPED        (1 << 2)
//page attribute
#define PAGE_PT_MAPED       (1 << 0)
#define PAGE_KERNEL_INIT    (1 << 1)
#define PAGE_DEVICE         (1 << 2)
#define PAGE_KERNEL         (1 << 3)
#define PAGE_SHARED         (1 << 4)

void init_memory(void);
unsigned long page_init(struct Page* page, unsigned long flags);
unsigned long* get_gdt(void);
struct Page* alloc_pages(int zones_select, int number, unsigned long flags);
#define flush_tlb()                 \
do                                  \
{                                   \
    unsigned long tmp;              \
    asm volatile                    \
    (                               \
        "movq %%cr3, %0          \n"\
        "movq %0, %%cr3          \n"\
        :"=r"(tmp)                  \
        :                           \
        :"memory"                   \
    );                              \
}while(0)

#define SIZEOF_LONG_ALIGN(size) ((size + sizeof(long) - 1) & ~(sizeof(long) - 1))
#define SIZEOF_INT_ALIGN(size) ((size + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define V_TO_2M(kaddr) (mem_structure.pages_struct + (V_TO_P(kaddr) >> PAGE_2M_SHIFT))
#define P_TO_2M(kaddr) (mem_structure.pages_struct + ((unsigned long)(kaddr) >> PAGE_2M_SHIFT))

struct SLAB_cache
{
    unsigned long size;
    unsigned long total_using;
    unsigned long total_free;
    struct SLAB* cache_pool;
    struct SLAB* dma_cache_pool;
    void* (*construct)(void* Vaddress, unsigned long arg);
    void* (*destruct)(void* Vaddress, unsigned long arg);
};
struct SLAB
{
    struct List list;
    struct Page* page;
    unsigned long using_count;
    unsigned long free_count;
    void* Vaddress;
    unsigned long color_length;
    unsigned long color_count;
    unsigned long* color_map;
};

extern struct SLAB_cache kmalloc_cache_size[16];
unsigned long slab_init(void);
struct SLAB_cache* slab_create(unsigned long size, void* (*construct)(void* Vaddress, unsigned long arg), void* (*destruct)(void* Vaddress, unsigned long arg), unsigned long arg);
void* kmalloc(unsigned long size, unsigned long flags);
unsigned long kfree(void* address);
void free_pages(struct Page* page, int number);
void* slab_malloc(struct SLAB_cache* slab_cache, unsigned long arg);
unsigned long slab_destroy(struct SLAB_cache* slab_cache);
unsigned long page_clean(struct Page * page);
unsigned long slab_free(struct SLAB_cache* slab_cache, void* address, unsigned long arg);
unsigned long get_page_attribute(struct Page* page);
unsigned long set_page_attribute(struct Page* page, unsigned long flags);
void pagetable_init();
unsigned long do_brk(unsigned long addr, unsigned long len);
#endif