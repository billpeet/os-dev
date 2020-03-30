#include "kernel.h"
#include "vga.h"
#include "stdio.h"
#include "idt.h"
#include "pong.h"
#include "shell.h"
#include "frame_allocator.h"
#include "types.h"
#include "boot_info.h"
#include "alloc.h"
#include "lba.h"
#include "task.h"
#include "x86.h"
#include "pi.h"
#include "paging.h"
#include "serial.h"
#include "console.h"
#include "keyboard.h"
#include <stdarg.h>

extern u16 code_selector;

void panic(char const *error_message, ...)
{
    cli();
    printf("PANIC!\n");
    va_list args;
    va_start(args, error_message);
    vprintf(error_message, args);
    while (1)
        hlt();
}

void reboot()
{
    u8 temp;

    cli();
    do
    {
        temp = inb(KEYBOARD_STATUS_PORT);
        if (temp & 0b10)
            inb(KEYBOARD_DATA_PORT);
    } while (temp & 1);

    outb(KEYBOARD_STATUS_PORT, 0xFE);
loop:
    hlt();
    goto loop;
}

static void other_main()
{
    printf("task0!\n");
    for (int i = 0; i < 100; i++)
    {
        printf("task0: %u\n", i);
        yield();
    }
    printf("other_main complete\n");
    kill();
}

static void other_main2_sub()
{
    printf("task1!\n");
    for (int i = 0; i < 100; i++)
    {
        printf("task1: %u\n", i);
        yield();
    }
    printf("task1 complete, dying now\n");
    kill();
}

static void other_main2()
{
    other_main2_sub();
}

static void sleeping_task()
{
    printf("Starting sleeping task!\n");
    sleep();
    printf("Waken up!\n");
    kill();
}

NORETURN void kmain(boot_info_t *boot_info)
{
    vga_clearScreen();
    printf("Welcome to PeetOS!\n");

    init_boot_info(boot_info);

    init_frame_allocator();
    init_interrupts();
    init_heap();
    init_serial();

    init_tasking();
    init_console();

    u64 flags = main_task.regs.flags;
    u64 cr3 = main_task.regs.cr3;

    // task_t task0 = create_task(other_main, flags, cr3);
    // schedule_task(task0);

    // task_t task1 = create_task(other_main2, flags, cr3);
    // schedule_task(task1);

    // task_t task2 = create_task(sleeping_task, flags, cr3);
    // schedule_task(task2);

    // task_t primes_task = create_task(get_primes, flags, cr3);
    // schedule_task(primes_task);

    task_t *shell_task = create_task(shell, flags, cr3);

    // printf("Running scheduler...\n");
    schedule();
}