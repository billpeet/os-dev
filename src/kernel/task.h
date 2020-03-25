#ifndef TASK_H
#define TASK_H

typedef enum taskstate_e
{
    UNUSED,
    RUNNABLE,
    IDLE
} taskstate;

#include "types.h"

typedef struct registers
{
    u64 rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
} registers_t;

typedef struct task
{
    registers_t regs;
    taskstate state;
    u32 id;
} task_t;

extern void init_tasking();

void display_current_task();

extern task_t create_task(void (*main)(), u64 flags, u64 *pagedir);
extern void create_task_with_stack(task_t *task, void (*main)(), u64 flags, u64 *pagedir, u64 rbp, u64 rsp);
extern void schedule_task(task_t task);

extern void sleep();
extern void kill();
extern void yield();
extern void switch_task(registers_t *next);

extern task_t *running_task;
extern task_t main_task;

extern __attribute__((noreturn)) void schedule();

#endif