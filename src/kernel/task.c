#include "task.h"
#include "paging.h"
#include "vga.h"
#include "x86.h"
#include "kernel.h"
#include "idt.h"

#define MAX_TASKS 64

task_t tasks[MAX_TASKS];

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

void create_task_with_stack(task_t *task, void (*main)(), u64 flags, u64 *pagedir, u64 rbp, u64 rsp)
{
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.flags = flags;
    task->regs.rip = (u64)main;
    task->regs.cr3 = (u64)pagedir;
    task->regs.rbp = rbp;
    task->regs.rsp = rsp;
    task->state = RUNNABLE;
}

task_t create_task(void (*main)(), u64 flags, u64 *pagedir)
{
    task_t task;
    u64 *stack = (u64 *)((u64)allocate_page() + 0x1000);
    *stack = (u64)main;
    create_task_with_stack(&task, main, flags, pagedir, (u64)stack, (u64)stack);
    return task;
}

void schedule_task(task_t task)
{
    for (u32 i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].state == UNUSED)
        {
            task.id = i;
            tasks[i] = task;
            return;
        }
    }
    printf("Exceeded task limit! Max. %u tasks supported\n", MAX_TASKS);
    panic(0);
}

// Remove running task from schedule and return to scheduler
void kill()
{
    if (running_task != &main_task)
    {
        printf("killing task %u\n", running_task->id);
        running_task->state = UNUSED;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
}

// Mark running task as idle and return to scheduler
void sleep()
{
    save_task(running_task); // Save so we can resume later
    if (running_task != &main_task)
    {
        printf("putting task %u to sleep\n", running_task->id);
        running_task->state = IDLE;
        running_task = &main_task;
        display_current_task();
        switch_task(&running_task->regs);
    }
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
    main_task.id = 9;

    running_task = &main_task;
    display_current_task();
}

// Yields control back to the scheduler
__attribute__((naked)) void yield()
{
    save_task(running_task);
    running_task = &main_task;
    display_current_task();
    switch_task(&running_task->regs);
}

__attribute__((naked)) void switch_to(task_t *task)
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
                printf("Switching to %u\n", tasks[i].id);
                switch_to(&tasks[i]);
            }
        }
    }
}