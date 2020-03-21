#ifndef TASK_H
#define TASK_H

#include "types.h"

typedef struct registers
{
    u64 rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
} registers_t;

typedef struct task
{
    registers_t regs;
    struct task *next;
} task_t;

extern void init_tasking();

void create_task(task_t *task, void (*main)(), u64 flags, u64 *pagedir);

extern void yield();
extern void switch_task(registers_t *old, registers_t *next);

#endif