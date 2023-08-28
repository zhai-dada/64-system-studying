#ifndef __UNISTD_H__
#define __UNISTD_H__

#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

int close(int fildes);
long read(int fildes, void *buf, long nbyte);
long write(int fildes, const void *buf, long nbyte);
long lseek(int fildes, long offset, int whence);

int fork(void);
int vfork(void);
unsigned long brk(unsigned long brk);
#endif
