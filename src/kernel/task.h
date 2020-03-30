#ifndef TASK_H
#define TASK_H

#include "gcc-attributes.h"

typedef enum taskstate_e
{
    // Unused task slot
    UNUSED,
    // 'Spare' task slot - stack has already been allocated
    SPARE,
    // Either running or waiting to run
    RUNNABLE,
    // Sleeping, waiting to be waken by another task
    IDLE,
    // Waiting for an interrupt
    WAITING
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
    u32 interrupt_id;
    u32 id;
    u64 time_spent;
} task_t;

extern void init_tasking();

extern void dump_tasks();

extern void display_current_task();

extern task_t *create_task(void (*main)(), u64 flags, u64 cr3);

extern void lock(void *);
extern void release(void *);
extern void sleep();
extern void wait_for_interrupt(u32 interrupt_id);
extern void wake(task_t *task);
extern void wake_interrupt(u32 interrupt_id);
extern void kill();
extern void yield();
extern void switch_task(registers_t *next);

extern task_t *running_task;
extern task_t main_task;

extern NORETURN void schedule();

#endif