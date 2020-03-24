#include "types.h"
#include "task.h"

__attribute__((always_inline)) static inline void save_rsp(task_t *task)
{
    asm volatile(
        "mov %%rsp, %0\n\t"
        : "=r"(task->regs.rsp)::);
    asm volatile(
        "mov %%rbp, %0\n\t"
        : "=r"(task->regs.rbp)::);
    asm volatile(
        "mov 0(%%rsp), %0\n\t"
        : "=r"(task->regs.rip)::);
    printf(" task %u rsp: %x\n", task->id, task->regs.rsp);
    // printf("task %u rip: %x\n", task->id, task->regs.rip);
    // u64 *rsp = (u64 *)task->regs.rsp;
    // task->regs.rip = *rsp;
    // printf("rip: 0x%x\n", task->regs.rip);
}

// Loads IDT
__attribute__((always_inline)) static inline void lidt()
{
    asm volatile("lidt [idtr]");
}

// Disables interrupts
__attribute__((always_inline)) static inline void cli()
{
    asm volatile("cli");
}

// Enables interrupts
__attribute__((always_inline)) static inline void sti()
{
    asm volatile("sti");
}