#include "idt.h"
#include "vga.h"

int isRunning;

void handle()
{
    unregister_handler(&handle);
    writeString("handled!\n");
    isRunning = 0;
}

void pong()
{
    isRunning = 1;
    register_handler(&handle);
    while (isRunning)
    {
    };
}