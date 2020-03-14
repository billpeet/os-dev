#include "shell.h"
#include "idt.h"
#include "vga.h"

char str[100];
int pos;

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
        str[i] = ' ';
    }
    pos = 0;
}

void shell_execute()
{
    writeChar('\n');
    if (starts_with(str, "pong"))
        writeString("you wanna play pong!\n");
    else
    {
        writeString("Unrecognised command '");
        writeString(str);
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
            str[--pos] = ' ';
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