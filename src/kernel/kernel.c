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

extern u16 code_selector;

void panic(u8 error_code)
{
    printf("PANIC!\n");
    printf("Error code %u\n", error_code);
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

void kmain(boot_info_t *boot_info)
{
    vga_clearScreen();
    printf("Welcome to PeetOS!\n");

    // while (1)
    //     asm volatile("hlt");

    init_boot_info(boot_info);

    init_frame_allocator();
    init_interrupts();
    init_heap();
    init_serial();

    init_tasking();

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

    task_t *shell_task = create_task(shell, main_task.regs.flags, cr3);
    printf("shell task cr3: 0x%x\n", shell_task->regs.cr3);

    printf("Running scheduler...\n");
    schedule();
    printf("Popped out of main!\n");
    panic(0);
}