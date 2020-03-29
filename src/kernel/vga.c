#include "vga.h"
#include "string.h"
#include "types.h"
#include <stdarg.h>

#define VGA_MAX VGA_WIDTH *VGA_HEIGHT

char virt_vga[VGA_MAX];
char *vidptr = (char *)0xb8000;

int current_pos = 0;

void setCharAtPos(const char c, int pos)
{
    vidptr[pos * 2] = c;
    vidptr[pos * 2 + 1] = 0x07;
}

void setCharAtPos_save(const char c, int pos)
{
    vidptr[pos * 2] = c;
    vidptr[pos * 2 + 1] = 0x07;
    virt_vga[pos * 2] = c;
    virt_vga[pos * 2 + 1] = 0x07;
}

void setChar(const char c, int line, int pos)
{
    setCharAtPos(c, (line * VGA_WIDTH) + pos);
}

void setChar_save(const char c, int line, int pos)
{
    setCharAtPos_save(c, (line * VGA_WIDTH) + pos);
}

void vga_clearScreen()
{
    for (int i = 0; i < VGA_MAX; i++)
        setCharAtPos_save(' ', i);
    current_pos = 0;
}

void vga_moveUp()
{
    for (int i = 1; i < VGA_HEIGHT; i++)
    {
        for (int j = 0; j < VGA_WIDTH; j++)
        {
            int prevPos = ((i - 1) * VGA_WIDTH) + j;
            int pos = (i * VGA_WIDTH) + j;
            virt_vga[prevPos * 2] = virt_vga[pos * 2];
            vidptr[prevPos * 2] = virt_vga[prevPos * 2];
            virt_vga[prevPos * 2 + 1] = virt_vga[pos * 2 + 1];
            vidptr[prevPos * 2 + 1] = virt_vga[prevPos * 2 + 1];

            setChar_save(' ', i, j); // Clear last line
        }
    }
    current_pos -= VGA_WIDTH;
}

void vga_writeChar(char c)
{
    if (c == '\n')
    {
        // Newline
        current_pos = VGA_WIDTH * (current_pos / VGA_WIDTH) + VGA_WIDTH;
        if (current_pos >= VGA_MAX)
            vga_moveUp();
    }
    else if (c == '\r')
        // Carriage return
        current_pos -= current_pos % VGA_WIDTH;
    else if (c == '\b')
        // Backspace
        setCharAtPos_save(' ', --current_pos);
    else
    {
        if (current_pos >= VGA_MAX)
            vga_moveUp();
        setCharAtPos_save(c, current_pos++);
    }
}