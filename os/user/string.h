#ifndef __STRING_H__
#define __STRING_H__
void *memcpy(void *From, void *To, long num);
void *memset(void *address, unsigned char c, long count);
char *strncpy(char *d, char *s, long count);
int strlen(char *s);
int strcmp(char *FirstPart, char *SecondPart);

#endif