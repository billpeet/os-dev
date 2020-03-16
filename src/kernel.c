#include "kernel.h"
#include "vga.h"
#include "idt.h"
#include "pong.h"
#include "shell.h"
#include "memory.h"

extern u16 code_selector;

void hlt()
{
    asm("hlt");
}

void kmain(void *args)
{
    clearScreen();
    writeString("Welcome to PeetOS\n");
    writeString("Total size: ");
    u32 *total_size = (u32 *)args;
    writeInt(*total_size);
    writeString("\n");
    init_interrupts();
    // init_paging();

    shell();
    while (1)
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