#include "vga.h"
#include "string.h"
#include "types.h"
#include <stdarg.h>

#define VGA_MAX VGA_WIDTH *VGA_HEIGHT

char virt_vga[VGA_MAX];

int current_pos = 0;

void setCharAtPos(const char c, int pos)
{
    char *vidptr = (char *)0xb8000;
    vidptr[pos * 2] = c;
    vidptr[pos * 2 + 1] = 0x07;
}

void setCharAtPos_save(const char c, int pos)
{
    char *vidptr = (char *)0xb8000;
    vidptr[pos * 2] = c;
    virt_vga[pos * 2] = c;
    vidptr[pos * 2 + 1] = 0x07;
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

void clearScreen()
{
    for (int i = 0; i < VGA_MAX; i++)
        setCharAtPos_save(' ', i);
    current_pos = 0;
}

void moveUp()
{
    char *vidptr = (char *)0xb8000;
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

void writeStatus(char const *str)
{
    for (int i = 0; i < VGA_WIDTH - 5; i++)
    {
        if (str[i] == '\0')
            break;
        setChar(str[i], VGA_HEIGHT - 1, i);
    }
}

void writeChar(char c)
{
    if (c == '\n')
    {
        current_pos = VGA_WIDTH * (current_pos / VGA_WIDTH) + VGA_WIDTH;
        if (current_pos >= VGA_MAX)
            moveUp();
    }
    else if (c == '\r')
    {
        current_pos -= current_pos % VGA_WIDTH;
    }
    else if (c == '\b')
    {
        setCharAtPos_save(' ', --current_pos);
    }
    else
    {
        if (current_pos >= VGA_MAX)
            moveUp();
        setCharAtPos_save(c, current_pos++);
    }
}

u8 getHexChar(u8 c)
{
    if (c >= 10)
        // Letter
        return c + 87;
    else
        // Number
        return c + 48;
}

void writeInt(u64 i)
{
    if (i < 10)
        writeChar(i + 48);
    else
    {
        char str[100];

        int j = 0;
        while (i > 0)
        {
            str[j] = (i % 10) + 48;
            i /= 10;
            j++;
        }
        str[j] = '\0';
        strrev(str);
        for (int i = 0; str[i] != '\0'; i++)
            writeChar(str[i]);
    }
}

void writeHexInt(u64 i)
{
    if (i < 16)
        writeChar(getHexChar((char)i));
    else
    {
        char str[100];

        int j = 0;
        while (i > 0)
        {
            str[j] = getHexChar((char)(i % 16));
            i /= 16;
            j++;
        }
        str[j] = '\0';
        strrev(str);
        for (int i = 0; str[i] != '\0'; i++)
            writeChar(str[i]);
    }
}

void writeSInt(long i)
{
    if (i < 0)
    {
        writeChar('-');
        i *= -1;
    }
    writeInt(i);
}

void writeString(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        writeChar(str[i]);
    }
}

void writeNewLine()
{
    writeChar('\n');
}

void printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0'; i++)
    {
        if (fmt[i] == '%')
        {
            switch (fmt[++i])
            {
            case 'd':
            case 'i':
                // signed int
                writeSInt(va_arg(args, long));
                break;
            case 'u':
            case 'f':
                // unsigned int
                writeInt(va_arg(args, u64));
                break;
            case 'x':
            case 'X':
                // hex
                writeHexInt(va_arg(args, u64));
                break;
            case 'c':
                // char
                writeChar(va_arg(args, int));
                break;
            case 's':
                // string
                writeString(va_arg(args, char *));
                break;
            case 'p':
                // pointer address
                writeHexInt((u64)va_arg(args, void *));
                break;
            default:
                writeChar('%');
                writeChar(fmt[i]);
                break;
            }
        }
        else
            writeChar(fmt[i]);
    }
}