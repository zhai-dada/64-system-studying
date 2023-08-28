#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include "lib.h"
#include "ptrace.h"
#include "vfs.h"

#define KEYBOARD_BUF_SIZE   100
extern struct file_operations keyboard_fops;
struct keyboard_input_buffer
{
    unsigned char* p_head;
    unsigned char* p_tail;
    int count;
    unsigned char buf[KEYBOARD_BUF_SIZE];
};
extern struct keyboard_input_buffer* p_kb;
#define PORT_KB_DATA    0x60
#define PORT_KB_STATUS  0x64
#define PORT_KB_CMD     0x64
#define KB_WRITE_CMD    0x60
#define KB_READ_CMD     0x20
#define KB_INIT_MODE    0x47
#define KB_STATUS_IN_BUF    0x02
#define KB_STATUS_OUT_BUF   0x01
#define wait_KB_write() while(io_in8(PORT_KB_STATUS) & KB_STATUS_IN_BUF)
#define wait_KB_read()  while(io_in8(PORT_KB_STATUS) & KB_STATUS_OUT_BUF)

#define SCAN_CODES  0x80
#define MAP_COLS    2
#define PAUSEBREAK  1
#define PRINTSCREEN 2
#define OTHERKEY    4
#define FLAG_BREAK  0x80

void keyboard_init(void);
void keyboard_handler(unsigned long nr, unsigned long parameter, struct registers_in_stack* reg);

#endif