#include "kernel.h"
#include "vga.h"
#include "idt.h"
#include "pong.h"
#include "shell.h"
#include "frame_allocator.h"
#include "types.h"
#include "boot_info.h"

extern u16 code_selector;

void hlt()
{
    asm("hlt");
}

void panic(u8 error_code)
{
    writeString("PANIC!\n");
    writeString("Error code ");
    writeInt(error_code);
    writeNewLine();
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
    writeString("Welcome to PeetOS\n");
    init_boot_info(boot_info);
    init_frame_allocator();

    init_interrupts();

    shell();
    while (1)
        hlt();
}