#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

void reboot(void);

void panic(u8 error_code);

#endif