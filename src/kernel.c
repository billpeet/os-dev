#include "vga.h"
#include "idt.h"
#include "pong.h"

void hlt()
{
    asm("hlt");
}

void shell()
{
    writeString(">");
    while (1)
    {
        hlt();
    }
}

void kmain(void)
{
    clearScreen();
    writeString("Welcome to PeetOS\n");
    init_interrupts();

    asm("int3");

    // pong();

    // writeString("exiting pong\n");

    int i = 0;
    int j = 1;
    int f = j / i;

    shell();
}