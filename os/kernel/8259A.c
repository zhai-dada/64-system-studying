#include "interrupt.h"
#include "linkage.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"
#include "lib.h"
#include "interrupt.h"
#include "8259A.h"

void init_8259A(void)
{
    int i = 0;
    for(i = 0x20; i <= 0xff; i++)
    {
        set_intr_gate(i, 2, interrupt[i - 0x20]);
    }
    color_printk(RED, BLACK, "8259A Init\n");
    io_out8(0x20, 0x11);
    io_out8(0x21, 0x20);
    io_out8(0x21, 0x04);
    io_out8(0x21, 0x01);

    io_out8(0xa0, 0x11);
    io_out8(0xa1, 0x28);
    io_out8(0xa1, 0x02);
    io_out8(0xa1, 0x01);

    io_out8(0x21, 0xfd);
    io_out8(0xa1, 0xff);
    sti();
    return;
}
void do_INT(struct registers_in_stack* regs, unsigned long nr)
{
	color_printk(RED,BLACK, "do_INT:%#04x\t", nr);
    char x = io_in8(0x60);
    color_printk(YELLOW, BLACK, "keycode:%#08x\n", x);
	io_out8(0x20, 0x20);
    return;
}