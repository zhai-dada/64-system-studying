#include "string.h"

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
void *memset(void *address, unsigned char c, long count)
{
    int d0, d1;
    unsigned long tmp = c * 0x0101010101010101UL;
    asm volatile(
        "cld                \n"
        "rep                \n"
        "stosq              \n"
        "testb $4, %b3      \n"
        "je 1f              \n"
        "stosl              \n"
        "1:\ttestb $2, %b3  \n"
        "je 2f              \n"
        "stosw              \n"
        "2:\ttestb $1, %b3  \n"
        "je 3f              \n"
        "stosb              \n"
        "3:                 \n"
        : "=&c"(d0), "=&D"(d1)
        : "a"(tmp), "q"(count), "0"(count / 8), "1"(address)
        : "memory");
}
char *strncpy(char *d, char *s, long count)
{
    asm volatile(
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
        :);
    return d;
}
int strlen(char *S)
{
    register int res;
    asm volatile(
        "cld            \n"
        "repne scasb    \n"
        "notl %0        \n"
        "decl %0        \n"
        : "=c"(res)
        : "D"(S), "a"(0), "0"(0xffffffff)
        :);
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