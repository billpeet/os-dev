#include "kernel.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "system/pi.h"
#include "system/tester.h"
#include "stdio.h"
#include "idt.h"
#include "shell.h"
#include "frame_allocator.h"
#include <stdint.h>
#include "boot_info.h"
#include "alloc.h"
#include "task.h"
#include "x86.h"
#include "paging.h"
#include "serial.h"
#include "console.h"
#include "gdt.h"
#include "fat.h"

extern uint16_t code_selector;

uint64_t ticks;

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
    uint8_t temp;

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
    exit(0);
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
    exit(0);
}

static void other_main2()
{
    other_main2_sub();
}

static void sleeping_task()
{
    printf("Starting sleeping task!\n");
    sleep();
    printf("Woken up!\n");
    exit(1);
}

NORETURN void kmain(boot_info_t *boot_info)
{
    vga_clearScreen();
    init_stdio();
    printf("Welcome to PeetOS!\n");

    init_boot_info(boot_info);

    init_frame_allocator();
    init_interrupts();
    init_heap();
    init_serial();

    init_tasking();
    init_console();

    init_drive(0);

    // tester();

    // dump_entry(&gdt64[0]);
    // dump_entry(&gdt64[1]);
    // dump_entry(&gdt64[2]);

    // init_ring3();

    uint64_t flags = main_task.regs.flags;
    uint64_t cr3 = main_task.regs.cr3;

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