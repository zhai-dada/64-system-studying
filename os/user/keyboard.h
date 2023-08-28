#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#define SCAN_CODES  0x80
#define MAP_COLS    2
#define PAUSEBREAK  1
#define PRINTSCREEN 2
#define OTHERKEY    4
#define FLAG_BREAK  0x80
extern int shift_left, shift_right, alt_left, alt_right, ctrl_right, ctrl_left;
extern unsigned char pausebreak_scancode[];
extern unsigned int keycode_map_normal[SCAN_CODES * MAP_COLS];
#endif