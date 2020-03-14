#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MAX VGA_WIDTH *VGA_HEIGHT

int current_pos = 0;

void setCharAtPos(const char c, int pos)
{
    char *vidptr = (char *)0xb8000;
    vidptr[pos * 2] = c;
    vidptr[pos * 2 + 1] = 0x07;
}

void setChar(const char c, int line, int pos)
{
    setCharAtPos(c, (line * VGA_WIDTH) + pos);
}

void clearScreen()
{
    for (int i = 0; i < VGA_MAX; i++)
        setCharAtPos(' ', i);
}

void writeStringAtPos(const char *str, int line, int pos)
{
    int initPos = line * VGA_WIDTH + pos;
    for (int i = 0; str[i] != '\0'; i++)
    {
        setCharAtPos(str[i], initPos + i);
    }
}

void writeString(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            current_pos = VGA_WIDTH * (current_pos / VGA_WIDTH) + VGA_WIDTH;
        }
        else
        {
            setCharAtPos(str[i], current_pos++);
        }
    }
}

void writeChar(char c)
{
    if (c == '\n')
        current_pos = VGA_WIDTH * (current_pos / VGA_WIDTH) + VGA_WIDTH;
    else
        setCharAtPos(c, current_pos++);
}