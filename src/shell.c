#include "shell.h"
#include "idt.h"
#include "vga.h"
#include "pong.h"
#include "alloc.h"
#include "string.h"
#include "memory.h"
#include "kernel.h"

char str[100];
int pos;

void shell_char(char c);

int starts_with(char *input, char const *check)
{
    char c;
    for (int i = 0; c = check[i]; i++)
    {
        if (!input[i] || input[i] != check[i])
            return 0;
    }
    return 1;
}

void shell_line_init()
{
    writeString("> ");
    for (int i = 0; i < 100; i++)
    {
        str[i] = '\0';
    }
    pos = 0;
}

void shell_execute()
{
    writeChar('\n');
    if (starts_with(str, "pong"))
    {
        writeString("Starting pong:\n");
        pong();
        writeString("done with pong\n");
        register_handler(shell_char);
    }
    else if (starts_with(str, "clear"))
    {
        clearScreen();
    }
    else if (starts_with(str, "mem"))
    {
        long *ptr = (long *)0xFFFFF;
        *ptr = 100;
        writeInt(*ptr);
        writeChar('\n');
        unsigned long *pd = (unsigned long *)0xFFFFF000;
        unsigned int pd_a = *pd;
        void *phys_ptr = get_physaddr(ptr);
        writeInt((long)phys_ptr);
        writeChar('\n');
    }
    else if (starts_with(str, "breakpoint"))
    {
        writeString("Triggering breakpoint exception:\n");
        asm("int3");
    }
    else if (starts_with(str, "page fault"))
    {
        writeString("Triggering page fault exception:\n");
        int *ptr = (int *)0xdeadbeef;
        *ptr = 1;
    }
    else if (starts_with(str, "divide by zero"))
    {
        writeString("Triggering divide by zero exception:\n");
        int i = 0;
        i /= 0;
    }
    else if (starts_with(str, "reboot"))
    {
        writeString("Rebooting...\n");
        reboot();
    }
    else if (starts_with(str, "alloc"))
    {
        writeString("Testing alloc:\n");
        int *ptr = malloc(sizeof(int));
        *ptr = 1;
        writeString("Address: ");
        writeInt((unsigned long)ptr);
        writeString(", value: ");
        writeInt(*ptr);
        writeChar('\n');
    }
    else
    {
        writeString("Unrecognised command '");
        char c;
        for (int i = 0; c = str[i] != '\0'; i++)
            writeChar(str[i]);
        writeString("'\n");
    }
    shell_line_init();
}

void shell_char(char c)
{
    if (c == '\b')
    {
        if (pos > 0)
        {
            str[--pos] = '\0';
            writeChar(c);
        }
    }
    else if (c == '\n')
        shell_execute();
    else
    {
        writeChar(c);
        str[pos++] = c;
    }
}

void shell(void)
{
    shell_line_init();
    register_handler(shell_char);

    while (1)
        asm("hlt");
}