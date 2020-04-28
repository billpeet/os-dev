#include "pong.h"
#include <stdio.h>
#include "../idt.h"
#include "../x86.h"

int isRunning;

// task_t *pong_task;

void handle()
{
    printf("handled!\n");
    // unregister_handler(&handle);
    isRunning = 0;
    yield();
}

void pong()
{
    printf("starting pong...\n");
    isRunning = 1;

    int_handler_t handler;
    handler.handler = handle;
    handler.task = running_task;

    sti();

    register_kbhandler(handler);
    while (isRunning)
    {
        hlt();
    };
}