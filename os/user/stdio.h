#ifndef __STDIO_H__
#define __STDIO_H__
#include <stdarg.h>
#define SEEK_SET 0 /* Seek relative to start-of-file */
#define SEEK_CUR 1 /* Seek relative to current position */
#define SEEK_END 2 /* Seek relative to end-of-file */

#define SEEK_MAX 3

int putstring(char *string);
int printf(const char *fmt, ...);
int sprintf(char * buf,const char * fmt,...);
int vsprintf(char * buf,const char *fmt, va_list args);

#endif
