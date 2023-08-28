#include "softirq.h"

void set_softirq_status(unsigned long status)
{
    softirq_status |= status;
    return;
}
unsigned long get_softirq_status()
{
    return softirq_status;
}
void register_softirq(int nr, void (*action)(void* data), void* data)
{
    softirq_vector[nr].action = action;
    softirq_vector[nr].data = data;
    return;
}
void unregister_softirq(int nr)
{
    softirq_vector[nr].action = NULL;
    softirq_vector[nr].data = NULL;
    return;
}
void softirq_init()
{
    softirq_status = 0;
    memset(softirq_vector, 0, sizeof(struct softirq) * 64);
    return;
}
void do_softirq()
{
    sti();
    int i = 0;
    
    for(i = 0; i < 64 && softirq_status; i++)
    {
        if(softirq_status & (1 << i))
        {
            softirq_vector[i].action(softirq_vector[i].data);
            softirq_status &= ~(1 << i);
        }
    }
    cli();
    return;
}