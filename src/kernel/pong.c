#include "idt.h"
#include "vga.h"

int isRunning;

void handle(char c)
{
    printf("handled!\n");
    unregister_handler(&handle);
    isRunning = 0;
}

void pong()
{
    printf("starting pong...\n");
    isRunning = 1;
    register_handler(&handle);
    while (isRunning)
    {
        //printf("x");
    };
}