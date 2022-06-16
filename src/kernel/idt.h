#ifndef IDT_H
#define IDT_H

#include <stddef.h>
#include <stdint.h>
#include "task.h"

#define TIMER_HANDLER_ID 32

void init_interrupts(void);

typedef struct int_handler
{
    task_t *task;
    void (*handler)(void);
} int_handler_t;

typedef struct interrupt_frame
{
    size_t rip;
    size_t code_segment;
    size_t flags;
    size_t rsp;
    size_t stack_segment;
} interrupt_frame_t;

int register_kbhandler(int_handler_t handler);
void unregister_kbhandler(int_handler_t handler);
int register_tmhandler(int_handler_t handler);
void unregister_tmhandler(int_handler_t handler);

#endif