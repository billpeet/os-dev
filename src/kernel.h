#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

void reboot(void);

void panic(u8 error_code);

extern void write_port(u16 port, u8 value);
extern void write_port_16(u16 port, u16 value);
extern u16 read_port(u16 port);

#endif