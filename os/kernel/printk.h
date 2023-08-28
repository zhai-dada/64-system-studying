#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>
#include "linkage.h"
#include "lib.h"
#include "spinlock.h"
//formal
#define ZEROPAD     1
#define SIGN        2
#define PLUS        4
#define SPACE       8
#define LEFT        16
#define SPECIAL     32
#define SMALL       64
 
#define is_digit(c) ((c) >= '0' && (c) <= '9')
//color
#define WHITE 	0x00ffffff		
#define BLACK 	0x00000000		
#define RED	    0x00ff0000		
#define ORANGE	0x00ff8000		
#define YELLOW	0x00ffff00		
#define GREEN	0x0000ff00		
#define BLUE	0x000000ff		
#define INDIGO	0x0000ffff		
#define PURPLE	0x008000ff		

struct position
{
	int XResolution;
	int YResolution;

	int XPosition;
	int YPosition;

	int XCharSize;
	int YCharSize;

	unsigned int * FB_addr;
	unsigned long FB_length;
	spinlock_t printk_lock;
};
extern struct position Pos;


void init_printk(void);
void VBE_buffer_init();
void putchar(unsigned int * fb,int Xsize,int x,int y,unsigned int FRcolor,unsigned int BKcolor,unsigned char font);
int skip_atoi(const char **s);
static char * number(char * str, long num, int base, int size, int precision ,int type);
int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char * fmt,...);
int vsprintf(char * buf,const char *fmt, va_list args);
void roll_screen(void);

#endif
