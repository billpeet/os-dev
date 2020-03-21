#include "task.h"
#include "paging.h"
#include "vga.h"

static task_t *running_task;
static task_t main_task;
static task_t other_task;

static void other_main()
{
    printf("Hello multitasking world\n");
    // yield();
    asm("hlt");
}

void create_task(task_t *task, void (*main)(), u64 flags, u64 *pagedir)
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
    u64 *stack = (u64 *)((u64)allocate_page() + 0x1000);
    *(stack - 1) = (u64)main;
    task->regs.rsp = (u64)(stack - 2);
    printf("stack at %u\n", (u64)stack - 2);
    task->next = 0;
}

void init_tasking()
{
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(main_task.regs.cr3)::"rax");
    asm volatile("pushf; mov (%%rsp), %%rax; mov %%rax, %0; popf;"
                 : "=m"(main_task.regs.flags)::"rax");
    printf("cr3: %x, flags: %x\n", main_task.regs.cr3, main_task.regs.flags);
    create_task(&other_task, other_main, main_task.regs.flags, (u64 *)main_task.regs.cr3);
    main_task.next = &other_task;
    other_task.next = &main_task;

    running_task = &main_task;
}

void dump_task(task_t *t)
{
    printf("rax: %u, flags: %x, rip: %x, cr3: %x, rsp: %x\n",
           t->regs.rax, t->regs.flags, t->regs.rip, t->regs.cr3, t->regs.rsp);
}

void yield()
{
    task_t *last = running_task;
    running_task = running_task->next;
    printf("Current:\n");
    dump_task(last);
    printf("Next:\n");
    dump_task(running_task);
    switch_task(&last->regs, &running_task->regs);
    printf("done\n");
}