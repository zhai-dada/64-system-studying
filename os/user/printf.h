#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdarg.h>
//formal
#define ZEROPAD     1
#define SIGN        2
#define PLUS        4
#define SPACE       8
#define LEFT        16
#define SPECIAL     32
#define SMALL       64
 
#define is_digit(c) ((c) >= '0' && (c) <= '9')


#define do_div(n, base)                   \
    ({                                    \
        int res;                          \
        asm(                              \
            "divq %%rcx"                  \
            : "=a"(n), "=d"(res)          \
            : "0"(n), "1"(0), "c"(base)); \
        res;                              \
    })
#endif