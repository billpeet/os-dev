#include "kernel.h"
#include "vga.h"
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

extern u16 code_selector;

void hlt()
{
    asm("hlt");
}

void panic(u8 error_code)
{
    printf("PANIC!\n");
    printf("Error code %u\n", error_code);
    hlt();
}

void reboot()
{
    u8 temp;

    asm volatile("cli");
    do
    {
        temp = read_port(KEYBOARD_STATUS_PORT);
        if (temp & 0b10)
            read_port(KEYBOARD_DATA_PORT);
    } while (temp & 1);

    write_port(KEYBOARD_STATUS_PORT, 0xFE);
loop:
    asm volatile("hlt");
    goto loop;
}

void kmain(boot_info_t *boot_info)
{
    clearScreen();
    printf("Welcome to PeetOS\n");
    init_boot_info(boot_info);
    init_frame_allocator();
    init_interrupts();
    outb(0x1f6, 0x40);
    init_heap();

    init_tasking();

    printf("Switching to task 1\n");
    // extern void print_task();
    // printf("Current rsp: \n");
    // print_task();
    // save_rsp(running_task);
    // u64 *ptr = (u64 *)running_task->regs.rsp;
    // printf("rip: %x\n", ptr[0]);
    u64 rip;
    asm volatile(
        "lea 0(%%rip), %0\n\t"
        : "=r"(rip)::);
    printf("rip: %x\n", rip);
    yield();
    printf("returned to main task\n");

    create_task(&shell_task, shell, main_task.regs.flags, (void *)main_task.regs.cr3);
    main_task.next = &shell_task;
    shell_task.next = &main_task;

    printf("switching to shell!\n");
    yield();

    // shell();

    // shell();
    while (1)
    {
        // yield();
        printf("x");
        asm("hlt");
    }
}