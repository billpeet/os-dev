#include "task.h"
#include "paging.h"
#include <stdio.h>
#include "drivers/vga.h"
#include "x86.h"
#include "kernel.h"
#include "idt.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_TASKS 64
#define MAX_LOCKS 64

task_t tasks[MAX_TASKS];

struct lock_table
{
    void *locks[MAX_LOCKS];
    uint16_t last_lock;
    uint16_t lock_count;
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

void create_task_with_stack(task_t *task, void (*main)(), uint64_t flags, uint64_t cr3, uint64_t rbp, uint64_t rsp)
{
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.flags = flags;
    task->regs.rip = (uint64_t)main;
    task->regs.cr3 = cr3;
    task->regs.rbp = rbp;
    task->regs.rsp = rsp;
    task->time_spent = 0;
    task->state = RUNNABLE;
}

task_t *create_task(void (*main)(), uint64_t flags, uint64_t cr3)
{
    task_t *task;
    size_t *stack;
    for (uint32_t i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == SPARE)
        {
            // Reuse existing stack
            task = &tasks[i];
            task->id = i;
            stack = (size_t *)task->regs.rbp;
            break;
        }
        else if (tasks[i].state == UNUSED)
        {
            // Create new stack
            task = &tasks[i];
            task->id = i;
            stack = (size_t *)((size_t)allocate_page() + 0x1000);
            break;
        }
    }
    if (task == NULL)
        panic("Exceeded task limit! Max. %u tasks supported\n", MAX_TASKS);

    // Load initial address into stack
    *stack = (size_t)main;
    create_task_with_stack(task, main, flags, cr3, (uint64_t)stack, (uint64_t)(stack));
    return task;
}

// Exits abnormally
void abort()
{
    if (running_task != &main_task)
    {
        running_task->state = SPARE;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Exits normally, with status code
void exit(int status)
{
    if (running_task->atexit)
        running_task->atexit();
    abort();
}

// Sets a function to run on program exit
int atexit(void (*func)(void))
{
    running_task->atexit = func;
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
        running_task->state = IDLE;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Mark running task was waiting for specified interrupt
NAKED void wait_for_interrupt(uint32_t interrupt_id)
{
    save_task(running_task);
    if (running_task != &main_task)
    {
        running_task->state = WAITING;
        running_task->interrupt_id = interrupt_id;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Wakes specified task
void wake(task_t *task)
{
    task->state = RUNNABLE;
}

// Wakes all tasks waiting for specified interrupt
void wake_interrupt(uint32_t interrupt_id)
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == WAITING && tasks[i].interrupt_id == interrupt_id)
            tasks[i].state = RUNNABLE;
    }
}

// Converts current context to the main_task
void init_tasking()
{
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(main_task.regs.cr3)::"rax");
    asm volatile("pushf; mov (%%rsp), %%rax; mov %%rax, %0; popf;"
                 : "=m"(main_task.regs.flags)::"rax");
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
void schedule()
{
    for (;;)
    {
        for (int i = 0; i < MAX_TASKS; i++)
        {
            if (tasks[i].state == RUNNABLE)
            {
                uint64_t ticks_before = ticks;
                // printf("switching to %u\n", tasks[i].id);
                switch_to(&tasks[i]);
                tasks[i].time_spent += ticks - ticks_before;
                // printf("back from %u\n", tasks[i].id);
            }
        }
    }
}