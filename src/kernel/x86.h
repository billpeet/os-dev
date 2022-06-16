#include <stddef.h>
#include <stdint.h>
#include "task.h"
#include "gcc-attributes.h"

// Saves the rdi, rsi, rbx, rsp, rbp & rip registers of the currently running task
ALWAYS_INLINE static inline void save_task(task_t *task)
{
    asm volatile(
        "mov %%rdi, %0\n\t"
        "mov %%rsi, %1\n\t"
        "mov %%rbx, %2\n\t"
        "mov %%rsp, %3\n\t"
        "mov %%rbp, %4\n\t"
        "mov 0(%%rsp), %5\n\t"
        "pushf\n\tpop %6\n\t"
        : "=r"(task->regs.rdi),
          "=r"(task->regs.rsi),
          "=r"(task->regs.rbx),
          "=r"(task->regs.rsp),
          "=r"(task->regs.rbp),
          "=r"(task->regs.rip),
          "=r"(task->regs.flags)::);
}

// Loads IDT
ALWAYS_INLINE static inline void lidt(void *idt, uint16_t size)
{
    volatile uint16_t idtr[3] ALIGNED(16);
    idtr[0] = size - 1;
    idtr[1] = (uint16_t)(size_t)idt;
    idtr[2] = (uint16_t)((size_t)idt >> 16);
    asm volatile("lidt (%0)" ::"r"(idtr));
}

// Triggers interrupt 3
ALWAYS_INLINE static inline void int3()
{
    asm volatile("int3");
}

// Disables interrupts
ALWAYS_INLINE static inline void cli()
{
    asm volatile("cli");
}

// Enables interrupts
ALWAYS_INLINE static inline void sti()
{
    asm volatile("sti");
}

ALWAYS_INLINE static inline void hlt()
{
    asm volatile("hlt");
}

// Gets byte from CPU port
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("in %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

// Gets word from CPU port
static inline uint16_t inw(uint16_t port)
{
    uint16_t data;
    asm volatile("in %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

// Recursively gets data from CPU port
static inline void insl(int port, void *addr, int cnt)
{
    asm volatile(
        "cld; rep insl"
        : "=D"(addr), "=c"(cnt)
        : "d"(port), "0"(addr), "1"(cnt)
        : "memory", "cc");
}

// Writes byte to CPU port
static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("out %0, %1" ::"a"(data), "d"(port));
}

// Writes word to CPU port
static inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("out %0, %1" ::"a"(data), "d"(port));
}

// Recursively writes data to CPU port
static inline void outsl(int port, const void *addr, int cnt)
{
    asm volatile(
        "cld; rep outsl"
        : "=S"(addr), "=c"(cnt)
        : "d"(port), "0"(addr), "1"(cnt)
        : "cc");
}

static inline uint8_t cpl()
{
    uint8_t cpl;
    asm volatile(
        "mov %%cs, %%ax\n\t"
        "and 3, %%ax\n\t"
        "mov %%ax, %0\n\t"
        : "=m"(cpl)::"ax");
    return cpl;
}