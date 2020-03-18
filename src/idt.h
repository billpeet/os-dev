#ifndef IDT_H
#define IDT_H

#include "types.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void init_interrupts(void);

typedef void (*keyboardHandlerFn)(char c);

void register_handler(keyboardHandlerFn handler);
void unregister_handler(keyboardHandlerFn handler);

#endif