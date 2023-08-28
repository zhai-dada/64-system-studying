#ifndef __TIME_H__
#define __TIME_H__
#include "lib.h"
#include "memory.h"
#include "printk.h"
#include "APIC.h"
#include "interrupt.h"
#include "spinlock.h"
#include "softirq.h"

struct time
{
    unsigned int second;    //00
    unsigned int minute;    //02
    unsigned int hour;      //04
    unsigned int week;      //06
    unsigned int day;       //07
    unsigned int month;     //08
    unsigned int year;      //09
    unsigned int century;   //32
};
extern struct time time;
#define BCD2BIN(value)  (((value) & 0xf) + ((value) >> 4) * 10)
struct timer_list
{
    struct List list;
    unsigned long expire_jiffies;
    void (*func)(void* data);
    void* data;
};
extern struct timer_list timer_list_head;

void get_cmos_time(struct time* time);
void do_timer(void* data);
void timer_init(void);
void test_timer(void* data);
void timer_init(void);
void del_timer(struct timer_list* timer);
void add_timer(struct timer_list* timer);
void init_timer(struct timer_list* timer, void (*func)(void* data), void* data, unsigned long expire_jiffies);

#endif