#ifndef __LINKAGE_H__
#define __LINKAGE_H__

#define L1_CACHE_BYTES 32 //???

#define asmlinkage __attribute__((regparm(0)))

#define __cacheline_aligned __attribute__((__aligned__(L1_CACHE_BYTES))) //???

#define SYMBOL_NAME(X) X
#define SYMBOL_NAME_STR(X) #X
#define SYMBOL_NAME_LABEL(X) X##:

#define ENTRY(name)            \
    .global SYMBOL_NAME(name); \
    SYMBOL_NAME_LABEL(name)

#endif