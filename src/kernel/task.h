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
    u32 id;
} task_t;

extern void init_tasking();

void display_current_task();

void create_task(task_t *task, void (*main)(), u64 flags, u64 *pagedir);
void create_task_with_stack(task_t *task, void (*main)(), u64 flags, u64 *pagedir, u64 rbp, u64 rsp);
void schedule_task(task_t *task);
void unschedule_task(task_t *task);

extern void yield_nosave();
extern void yield();
extern void switch_task(registers_t *old, registers_t *next);
extern void switch_task_no_save(registers_t *old, registers_t *next);

extern task_t *running_task;
extern task_t main_task;

#endif