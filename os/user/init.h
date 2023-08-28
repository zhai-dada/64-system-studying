#ifndef __TEST_H__
#define __TEST_H__

struct	buildincmd
{
	char *name;
	int (*function)(int,char**);
};
#define NULL ((void*)0)
extern struct buildincmd shell_internal_cmd[];

extern char *current_dir;

#endif