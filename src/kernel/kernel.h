#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"
#include "gcc-attributes.h"

void reboot(void);

NORETURN void panic(char const *error_message, ...);

u64 ticks;

#endif