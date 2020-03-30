#ifndef IDT_H
#define IDT_H

#include "types.h"
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
    u64 rip;
    u64 code_segment;
    u64 flags;
    u64 rsp;
    u64 stack_segment;
} interrupt_frame_t;

int register_kbhandler(int_handler_t handler);
void unregister_kbhandler(int_handler_t handler);
int register_tmhandler(int_handler_t handler);
void unregister_tmhandler(int_handler_t handler);

#endif