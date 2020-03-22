#include "task.h"
#include "paging.h"
#include "vga.h"

task_t *running_task;
task_t main_task;
task_t task1;
task_t task2;

static int task_id = 1;

static void other_main()
{
    printf("In other task\n");
    printf("Switching back to main\n");
    yield();
}

static void other_main2()
{
    printf("task2\n");
    yield();
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
    *stack = task->regs.rip;
    task->regs.rsp = (u64)stack;
    task->next = 0;
    task->id = task_id++;
}

void init_tasking()
{
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(main_task.regs.cr3)::"rax");
    asm volatile("pushf; mov (%%rsp), %%rax; mov %%rax, %0; popf;"
                 : "=m"(main_task.regs.flags)::"rax");
    printf("cr3: %x, flags: %x\n", main_task.regs.cr3, main_task.regs.flags);
    create_task(&task1, other_main, main_task.regs.flags, (u64 *)main_task.regs.cr3);
    create_task(&task2, other_main2, main_task.regs.flags, (u64 *)main_task.regs.cr3);
    // main_task.regs.rip = (u64)other_main2;
    main_task.id = 1;
    main_task.next = &task1;
    task1.next = &main_task;
    task1.id = 2;
    // task2.next = &task1;
    // task2.id = 3;

    // extern void some_func(task_t * t);
    // main_task.regs.rsp = 0x1111;
    // some_func(&main_task);

    running_task = &main_task;
}

void dump_task(task_t *t)
{
    printf("rax: %u, flags: %x, rip: %x, cr3: %x, rsp: %x\n",
           t->regs.rax, t->regs.flags, t->regs.rip, t->regs.cr3, t->regs.rsp);
}

void yield()
{
    extern void save_task(task_t * t);
    save_task(running_task);
    u64 rip;
    task_t *last = running_task;
    running_task = running_task->next;
    printf("switching from %u to %u: last=%p, run=%p, run addr %p\n", last->id, running_task->id, &last->regs, &running_task->regs, running_task->regs.rip);

    // if (running_task->next != NULLPTR)
    // {
    switch_task(&last->regs, &running_task->regs);
    // }
}
// 0x108f28