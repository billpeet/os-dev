#include "idt.h"
#include "vga.h"

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
    // create_task(pong_task, handle, running_task->regs.flags, running_task->regs.cr3);
    isRunning = 1;

    int_handler_t handler;
    handler.handler = handle;
    handler.task = running_task;

    asm("sti");

    register_kbhandler(handler);
    while (isRunning)
    {
        // printf("x");
        asm("hlt");
    };
}