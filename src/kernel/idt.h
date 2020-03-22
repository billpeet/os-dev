#ifndef IDT_H
#define IDT_H

#include "types.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void init_interrupts(void);

typedef void (*keyboardHandlerFn)(char c);

typedef struct exception_frame
{
    u64 instruction_pointer;
    u64 code_segment;
    u64 cpu_flags;
    u64 stack_pointer;
    u64 stack_segment;
} exception_frame_t;

void register_handler(keyboardHandlerFn handler);
void unregister_handler(keyboardHandlerFn handler);

#endif