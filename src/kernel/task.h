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

#include <stdint.h>

typedef struct registers
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, rip, flags, cr3;
} registers_t;

typedef struct task
{
    registers_t regs;
    taskstate state;
    uint32_t interrupt_id;
    uint32_t id;
    uint64_t time_spent;
    void (*atexit)(void);
} task_t;

extern void init_tasking();

extern void dump_tasks();

extern void display_current_task();

extern task_t *create_task(void (*main)(), uint64_t flags, uint64_t cr3);

extern void lock(void *);
extern void release(void *);
extern void sleep();
extern void wait_for_interrupt(uint32_t interrupt_id);
extern void wake(task_t *task);
extern void wake_interrupt(uint32_t interrupt_id);
extern void yield();
extern void switch_task(registers_t *next);

extern task_t *running_task;
extern task_t main_task;

extern NORETURN void schedule();

#endif