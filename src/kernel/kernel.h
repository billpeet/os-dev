#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include "gcc-attributes.h"

void reboot(void);

NORETURN void panic(char const *error_message, ...);

extern uint64_t ticks;

#endif