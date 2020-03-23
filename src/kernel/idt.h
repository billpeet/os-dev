#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "task.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void init_interrupts(void);

typedef void (*keyboardHandlerFn)(char c);

typedef struct int_handler
{
    task_t *task;
    keyboardHandlerFn handler;
} int_handler_t;

typedef struct interrupt_frame
{
    u64 *instruction_pointer;
    u64 code_segment;
    u64 cpu_flags;
    u64 *stack_pointer;
    u64 stack_segment;
} interrupt_frame_t;

void register_handler(int_handler_t handler);
void unregister_handler(int_handler_t handler);

#endif