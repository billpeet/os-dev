#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

extern void init_serial(void);

extern uint8_t serial_received();

extern uint8_t read_serial();

extern uint8_t transmit_empty();

extern void write_serial(uint8_t data);

#endif