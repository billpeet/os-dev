#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

void reboot(void);

void panic(u8 error_code);

static inline u8 inb(u16 port)
{
    u8 data;
    asm volatile("in %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline void insl(int port, void *addr, int cnt)
{
    asm volatile(
        "cld; rep insl"
        : "=D"(addr), "=c"(cnt)
        : "d"(port), "0"(addr), "1"(cnt)
        : "memory", "cc");
}

static inline u16 inw(u16 port)
{
    u16 data;
    asm volatile("in %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline void outb(u16 port, u8 data)
{
    asm volatile(
        "out %0, %1"
        :
        : "a"(data), "d"(port));
}

static inline void outw(u16 port, u16 data)
{
    asm volatile(
        "out %0, %1"
        :
        : "a"(data), "d"(port));
}

static inline void outsl(int port, const void *addr, int cnt)
{
    asm volatile(
        "cld; rep outsl"
        : "=S"(addr), "=c"(cnt)
        : "d"(port), "0"(addr), "1"(cnt)
        : "cc");
}

#endif