#ifndef __LIB_H__
#define __LIB_H__

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE
#define NULL 0

#define PACKED __attribute__((packed))

#define container_of(ptr, type, type_member)                                    \
({                                                                              \
    typeof(((type *)0)->type_member) *p = (ptr);                                \
    (type *)((unsigned long)p - (unsigned long)&(((type *)0)->type_member));    \
})
#define do_div(n, base)                 \
({                                      \
    int res;                            \
    asm                                 \
    (                                   \
        "divq %%rcx"                    \
        : "=a"(n), "=d"(res)            \
        : "0"(n), "1"(0), "c"(base)     \
    );                                  \
    res;                                \
})
struct List
{
    struct List* prev;
    struct List* next;
};
void list_init(struct List* list)
{
    list->prev = list;
    list->next = list;
    return;
}
void list_add_behind(struct List* list, struct List* newn)
{
    newn->next = list->next;
    newn->prev = list;
    newn->next->prev = newn;
    list->next = newn;
    return;
}
void list_add_before(struct List* list, struct List* newn)
{
    list->prev->next = newn;
    newn->prev = list->prev;
    list->prev = newn;
    newn->next = list;
    return;
}
void list_delete(struct List* list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;
    return;
}
long list_is_empty(struct List* list)
{
    if(list->next == list && list->prev == list)
    {
        return 1;
    }
    return 0;
}
struct List* list_prev(struct List* list)
{
    if(list->prev != NULL)
    {
        return list->prev;
    }
    return NULL;
}
struct List* list_next(struct List* list)
{
    if(list->next != NULL)
    {
        return list->next;
    }
    return NULL;
}
void* memcpy(void* From, void* To, long num)
{
    int d0, d1, d2;
    asm volatile
    (
        "cld    \n"
        "rep movsq\n"
        "testb $4, %b4\n"
        "jz 1f\n"
        "movsl\n"
        "1:\n"
        "testb $2, %b4\n"
        "jz 2f\n"
        "movsw\n"
        "2:\n"
        "testb $1, %b4\n"
        "jz 3f\n"
        "movsb\n"
        "3:\n"
        :"=&c"(d0), "=&D"(d1), "=&S"(d2)
        :"0"(num / 8), "q"(num), "1"(To), "2"(From)
        :"memory"
    );
    return To;
}
void* memset(void *address, unsigned char c, long count)
{
    int d0, d1;
    unsigned long tmp = c * 0x0101010101010101UL;
    asm volatile
    (
        "cld                \n"
        "rep                \n"
        "stosq              \n"
        "testb $4, %b3      \n"
        "je 1f              \n"
        "stosl              \n"
        "1:                 \n"
        "testb $2, %b3      \n"
        "je 2f              \n"
        "stosw              \n"
        "2:                 \n"
        "testb $1, %b3      \n"
        "je 3f              \n"
        "stosb              \n"
        "3:                 \n"
        : "=&c"(d0), "=&D"(d1)
        : "a"(tmp), "q"(count), "0"(count / 8), "1"(address)
        : "memory"
    );
}
char *strncpy(char *d, char *s, long count)
{
    asm volatile
    (
        "cld                \n"
        "1:                 \n"
        "decq %2            \n"
        "js 2f              \n"
        "lodsb              \n"
        "stosb              \n"
        "testb %%al, %%al   \n"
        "jne 1b             \n"
        "rep                \n"
        "stosb              \n"
        "2:                 \n"
        :
        : "S"(s), "D"(d), "c"(count)
        :"memory"
    );
    return d;
}
int strlen(char *S)
{
    register int res;
    asm volatile
    (
        "cld            \n"
        "repne scasb    \n"
        "notl %0        \n"
        "decl %0        \n"
        : "=c"(res)
        : "D"(S), "a"(0), "0"(0xffffffff)
        :"memory"
    );
    return res;
}
int strcmp(char* FirstPart, char* SecondPart)
{
	register int res;
	asm	volatile
    (	
        "cld	\n"
		"1:	\n"
		"lodsb	\n"
		"scasb	\n"
		"jne	2f	\n"
		"testb	%%al,	%%al	\n"
		"jne	1b	\n"
		"xorl	%%eax,	%%eax	\n"
		"jmp	3f	\n"
		"2:	\n"
        "movl	$1,	%%eax	\n"
		"jl	3f	\n"
		"negl	%%eax	\n"
		"3:	\n"
		:"=a"(res)
		:"D"(FirstPart),"S"(SecondPart)
		:					
	);
	return res;
}
#define nop()       \
    asm volatile    \
    (               \
        "nop\n"     \
        :           \
        :           \
        : "memory"  \
    );

#define hlt()       \
    asm volatile    \
    (               \
        "hlt\n"     \
        :           \
        :           \
        : "memory"  \
    );
#define sti()       \
    asm volatile    \
    (               \
        "sti\n"     \
        :           \
        :           \
        : "memory"  \
    );
#define cli()       \
    asm volatile    \
    (               \
        "cli\n"     \
        :           \
        :           \
        : "memory"  \
    );
#define io_mfence() \
    asm volatile    \
    (               \
        "mfence\n"  \
        :           \
        :           \
        :"memory"   \
    )
unsigned char io_in8(unsigned short port)
{
    unsigned char ret = 0;
    asm volatile
    (
        "inb %%dx, %0       \n"
        "mfence             \n"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}
unsigned int io_in32(unsigned short port)
{
    unsigned int ret = 0;
    asm volatile
    (
        "inl %%dx, %0       \n"
        "mfence             \n"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}
void io_out8(unsigned short port, unsigned char value)
{
    asm volatile
    (
        "outb %0, %%dx      \n"
        "mfence             \n"
        :
        : "a"(value), "d"(port)
        : "memory"
    );
    return;
}
void io_out32(unsigned short port, unsigned int value)
{
    asm volatile
    (
        "outl %0, %%dx      \n"
        "mfence             \n"
        :
        : "a"(value), "d"(port)
        : "memory"
    );
    return;
}
unsigned long get_rsp(void)
{
    unsigned long rsp;
    asm volatile
    (
        "movq %%rsp, %0\n"
        :"=r"(rsp)
        :
        :"memory"
    );
    return rsp;
}
unsigned long rdmsr(unsigned long address)
{
    unsigned long r1, r2;
    asm volatile
    (
        "rdmsr\n"
        :"=d"(r1), "=a"(r2)
        :"c"(address)
        :"memory"
    );
    return (unsigned long)((r1 << 32) | r2);
}
void wrmsr(unsigned long address, unsigned long value)
{
    asm volatile
    (
        "wrmsr\n"
        :
        :"d"(value >> 32), "a"(value & 0xffffffff), "c"(address)
        :"memory"
    );
    return;
}

void cpuid(unsigned int mainleaf, unsigned int subleaf, unsigned int* a, unsigned int* b, unsigned int* c, unsigned int* d)
{
    asm volatile
    (
        "cpuid\n"
        :"=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        :"0"(mainleaf), "2"(subleaf)
        :"memory"
    );
    return;
}
unsigned long get_rflags()
{
	unsigned long tmp = 0;
    asm volatile
    (
        "pushfq     \n"
        "movq 0(%%rsp), %0   \n"
        "popfq      \n"
        :"=r"(tmp)
        :
        :"memory"
    );
	return tmp;
}
long verify_area(unsigned char* addr, unsigned long size)
{
	if(((unsigned long)addr + size) <= (unsigned long)0x00007fffffffffff)
	{
        return 1;
    }
	return 0;
}

long copy_from_user(void* from, void* to, unsigned long size)
{
	unsigned long d0,d1;
	if(!verify_area((unsigned char*)from, size))
	{
        return 0;
    }
	asm volatile
    (
        "rep	\n"
		"movsq	\n"
		"movq	%3,	%0	\n"
		"rep	\n"
		"movsb	\n"
		:"=&c"(size),"=&D"(d0),"=&S"(d1)
		:"r"(size & 7),"0"(size / 8),"1"(to),"2"(from)
		:"memory"
	);
	return size;
}

long copy_to_user(void* from, void* to, unsigned long size)
{
	unsigned long d0,d1;
	if(!verify_area((unsigned char*)to, size))
	{
        return 0;
    }
	asm volatile
    (
        "rep	\n"
		"movsq	\n"
		"movq	%3,	%0	\n"
		"rep	\n"
		"movsb	\n"
		:"=&c"(size),"=&D"(d0),"=&S"(d1)
		:"r"(size & 7),"0"(size / 8),"1"(to),"2"(from)
		:"memory"
	);
	return size;
}

long strncpy_from_user(void* from, void* to, unsigned long size)
{
	if(!verify_area((unsigned char*)from,size))
	{
        return 0;
    }
	strncpy((char*)to, (char*)from, size);
	return	size;
}

long strnlen_user(void* src, unsigned long maxlen)
{
	unsigned long size = strlen((char*)src);
	if(!verify_area((unsigned char*)src, size))
	{
        return 0;
    }
	return size <= maxlen ? size : maxlen;
}
#define port_insw(port, buffer, nr)	    \
    asm volatile                        \
    (                                   \
        "cld\n"                         \
        "rep insw\n"                    \
        "mfence\n"                      \
        :                               \
        :"d"(port),"D"(buffer),"c"(nr)  \
        :"memory"                       \
    )

#define port_outsw(port, buffer, nr)	\
    asm volatile                        \
    (                                   \
        "cld\n"                         \
        "rep outsw\n"                   \
        "mfence\n"                      \
        :                               \
        :"d"(port),"S"(buffer),"c"(nr)  \
        :"memory"                       \
    )
#endif