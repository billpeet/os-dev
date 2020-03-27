#include "task.h"
#include "paging.h"
#include "stdio.h"
#include "vga.h"
#include "x86.h"
#include "kernel.h"
#include "idt.h"
#include <stddef.h>
#include <stdint.h>

#define NAKED __attribute__((naked))

#define MAX_TASKS 64
#define MAX_LOCKS 64

task_t tasks[MAX_TASKS];

struct lock_table
{
    void *locks[MAX_LOCKS];
    u16 last_lock;
    u16 lock_count;
} locks;

task_t *running_task;
task_t main_task;

static int task_id = 1;

void display_current_task()
{
    if (running_task == &main_task)
        setChar('M', VGA_HEIGHT - 1, VGA_WIDTH - 1);
    else
        setChar(running_task->id + 48, VGA_HEIGHT - 1, VGA_WIDTH - 1);
}

void dump_tasks()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state != UNUSED && tasks[i].state != SPARE)
            printf("Task %u, state %u, %u%% CPU\n", tasks[i].id, tasks[i].state, 100 * tasks[i].time_spent / ticks);
    }
}

void create_task_with_stack(task_t *task, void (*main)(), u64 flags, u64 cr3, u64 rbp, u64 rsp)
{
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.flags = flags;
    task->regs.rip = (u64)main;
    task->regs.cr3 = cr3;
    task->regs.rbp = rbp;
    task->regs.rsp = rsp;
    task->time_spent = 0;
    task->state = RUNNABLE;
}

task_t *create_task(void (*main)(), u64 flags, u64 cr3)
{
    task_t *task;
    u64 *stack;
    for (u32 i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == SPARE)
        {
            task = &tasks[i];
            task->id = i;
            stack = (u64 *)task->regs.rbp;
            break;
        }
        else if (tasks[i].state == UNUSED)
        {
            task = &tasks[i];
            task->id = i;
            stack = (u64 *)((u64)allocate_page() + 0x1000);
            break;
        }
    }
    if (task == NULL)
    {
        printf("Exceeded task limit! Max. %u tasks supported\n", MAX_TASKS);
        panic(0);
    }

    // Load initial address into stack
    *stack = (u64)main;
    create_task_with_stack(task, main, flags, cr3, (u64)stack, (u64)stack);
    return task;
}

// Remove running task from schedule and return to scheduler
void kill()
{
    if (running_task != &main_task)
    {
        // printf("Killing task %u\n", running_task->id);
        // unmap_page(running_task->regs.rsp - 0x1000);
        running_task->state = SPARE;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Locks memory address
void lock(void *ptr)
{
    for (int i = 0; i < locks.last_lock + 1; i++)
    {
        if (locks.locks[i] = NULL)
        {
            // Free slot, lets use it
            locks.locks[i] = ptr;
            locks.last_lock = i;
            return;
        }
        else if (locks.locks[i] == ptr)
        {
            // This memory address is already locked - spin until the other process lets go
            while (locks.locks[i] == ptr)
                yield();
            // Lock again
            locks.locks[i] == ptr;
            return;
        }
    }
}

// Releases lock on memory address
void release(void *ptr)
{
    for (int i = 0; i < MAX_LOCKS; i++)
    {
        if (locks.locks[i] == ptr)
        {
            locks.locks[i] = NULL;
            if (i >= locks.last_lock)
                locks.last_lock = i - 1;
        }
    }
}

// Mark running task as idle and return to scheduler
NAKED void sleep()
{
    save_task(running_task); // Save so we can resume later
    if (running_task != &main_task)
    {
        // printf("putting task %u to sleep\n", running_task->id);
        running_task->state = IDLE;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Wakes specified task
void wake(task_t *task)
{
    // printf("Waking task %u\n", task->id);
    task->state = RUNNABLE;
}

// Converts current context to the main_task
void init_tasking()
{
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(main_task.regs.cr3)::"rax");
    asm volatile("pushf; mov (%%rsp), %%rax; mov %%rax, %0; popf;"
                 : "=m"(main_task.regs.flags)::"rax");
    printf("cr3: %x, flags: %x\n", main_task.regs.cr3, main_task.regs.flags);
    main_task.state = RUNNABLE;

    running_task = &main_task;
    display_current_task();
}

// Yields control back to the scheduler
NAKED void yield()
{
    save_task(running_task);
    running_task = &main_task;
    display_current_task();
    switch_task(&running_task->regs);
}

// Switches to specified task
NAKED void switch_to(task_t *task)
{
    save_task(running_task);
    running_task = task;
    display_current_task();
    switch_task(&running_task->regs);
}

// Infinite looping scheduler, scheduling each task one at a time
__attribute__((noreturn)) void schedule()
{
    for (;;)
    {
        for (int i = 0; i < MAX_TASKS; i++)
        {
            if (tasks[i].state == RUNNABLE)
            {
                u64 ticks_before = ticks;
                // printf("switching to %u\n", tasks[i].id);
                switch_to(&tasks[i]);
                tasks[i].time_spent += ticks - ticks_before;
                // printf("back from %u\n", tasks[i].id);
            }
            // else
            //     printf("%u NR ", tasks[i].id);
        }
    }
}