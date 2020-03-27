#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

extern void init_serial(void);

extern u8 serial_received();

extern u8 read_serial();

extern u8 transmit_empty();

extern void write_serial(u8 data);

#endif