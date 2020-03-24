#include "task.h"
#include "paging.h"
#include "vga.h"
#include "x86.h"
#include "kernel.h"
#include "idt.h"

task_t *running_task;
task_t main_task;

task_t *head;
task_t *tail;

static int task_id = 1;

void display_current_task()
{
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
    task->next = 0;
    task->id = task_id++;
}

void create_task(task_t *task, void (*main)(), u64 flags, u64 *pagedir)
{
    u64 *stack = (u64 *)((u64)allocate_page() + 0x1000);
    create_task_with_stack(task, main, flags, pagedir, (u64)stack, (u64)stack);
}

void schedule_task(task_t *task)
{
    if (tail == NULLPTR)
    {
        head = task;
        tail = head;
        head->next == NULLPTR;
        return;
    }

    // Append to end
    tail->next = task;
    tail = task;
    tail->next = head;
}

void unschedule_task(task_t *task)
{
    if (head == task)
    {
        if (head == tail)
        {
            // Removing only task
            head = NULLPTR;
            tail = NULLPTR;
        }
        else
        {
            // Removing head
            head == task->next;
            tail->next = head;
        }
        return;
    }

    task_t *curr = head;
    while (curr->next != NULLPTR && curr->next != task)
        curr = curr->next;
    if (curr->next == NULLPTR)
    {
        printf("Task %u not found!\n", task->id);
        panic(0);
    }

    if (task == tail)
    {
        // Pop off the end
        tail = curr;
        tail->next = head;
    }
    else
    {
        // Remove from the middle
        curr->next = task->next;
    }
}

void switch_tasks()
{
    task_t *last = running_task;
    running_task = running_task->next;
    display_current_task();
    switch_task(&last->regs, &running_task->regs);
}

void init_tasking()
{
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(main_task.regs.cr3)::"rax");
    asm volatile("pushf; mov (%%rsp), %%rax; mov %%rax, %0; popf;"
                 : "=m"(main_task.regs.flags)::"rax");
    printf("cr3: %x, flags: %x\n", main_task.regs.cr3, main_task.regs.flags);

    schedule_task(&main_task);
    running_task = &main_task;
    display_current_task();
}

void dump_task(task_t *t)
{
    printf("rax: %u, flags: %x, rip: %x, cr3: %x, rsp: %x\n",
           t->regs.rax, t->regs.flags, t->regs.rip, t->regs.cr3, t->regs.rsp);
}

__attribute__((naked)) void yield_nosave()
{
    task_t *last = running_task;
    running_task = running_task->next;
    display_current_task();
    switch_task(&last->regs, &running_task->regs);
}

__attribute__((naked)) void yield()
{
    // Save existing task's RSP/RBP
    save_rsp(running_task);
    task_t *last = running_task;
    running_task = running_task->next;
    display_current_task();
    switch_task(&last->regs, &running_task->regs);
}
