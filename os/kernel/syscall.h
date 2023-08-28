#ifndef __SYSCALL_H__
#define __SYS_CALL_H
#include "unistd.h"

SYS_CALL(sys_printf_nr, sys_printf)
SYS_CALL(sys_open_nr, sys_open)
SYS_CALL(sys_close_nr, sys_close)
SYS_CALL(sys_read_nr, sys_read)
SYS_CALL(sys_write_nr, sys_write)
SYS_CALL(sys_lseek_nr, sys_lseek)
SYS_CALL(sys_fork_nr, sys_fork)
SYS_CALL(sys_vfork_nr, sys_vfork)
SYS_CALL(sys_brk_nr, sys_brk)
SYS_CALL(sys_device_open_nr, sys_device_openkeyboard)
#endif