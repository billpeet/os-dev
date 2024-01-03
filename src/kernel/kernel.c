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

NORETURN void kmain(boot_info_t *boot_info)
{
    vga_clearScreen();
    printf("Welcome to PeetOS!\n");

    init_boot_info(boot_info);
    // init_stdio();
    init_frame_allocator();
    init_interrupts();
    init_heap();
    init_serial();
    init_console();
    init_drive(0);

    init_tasking();

    // tester();

    uint64_t flags = main_task.regs.flags;
    uint64_t cr3 = main_task.regs.cr3;

    task_t *shell_task = create_task(shell, flags, cr3);

    // printf("Running scheduler...\n");
    schedule();
}