#include "vga.h"
#include "idt.h"
#include "pong.h"
#include "shell.h"
#include "memory.h"

void hlt()
{
    asm("hlt");
}

void kmain(void)
{
    clearScreen();
    writeString("Welcome to PeetOS\n");
    init_interrupts();
    init_paging();

    // pong();

    // asm("int3");

    // pong();

    // writeString("exiting pong\n");

    // int i = 0;
    // int j = 1;
    // int f = j / i;

    shell();
    // while (1)
    //     ;
}