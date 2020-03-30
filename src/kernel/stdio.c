#include "stdio.h"
#include "console.h"
#include "vga.h"
#include <stdarg.h>
#include "types.h"
#include <string.h>
#include "serial.h"
#include "task.h"
#include "idt.h"
#include "keyboard.h"

#define HEXCHAR(c) c >= 10 ? c + 87 : c + 48

char readChar()
{
    while (char_available() == 0)
        wait_for_interrupt(KEYBOARD_HANDLER_ID);
    return dequeue();
}

void putChar(char c)
{
    write_serial(c);
    vga_writeChar(c);
}

int writeString(const char *str)
{
    int i = 0;
    for (i = 0; str[i] != '\0'; i++)
        putChar(str[i]);
    return i - 1;
}

int writeInt(u64 i)
{
    if (i < 10)
    {
        putChar(i + 48);
        return 1;
    }
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
        return writeString(str);
    }
}

int writeSInt(long i)
{
    int cnt = 0;
    if (i < 0)
    {
        putChar('-');
        cnt++;
        i *= -1;
    }
    return cnt + writeInt(i);
}

int writeHexInt(u64 i)
{
    if (i < 16)
    {
        putChar(HEXCHAR((char)i));
        return 1;
    }
    else
    {
        char str[100];

        int j = 0;
        while (i > 0)
        {
            str[j] = HEXCHAR((char)(i % 16));
            i /= 16;
            j++;
        }
        str[j] = '\0';
        strrev(str);
        return writeString(str);
    }
}

int vprintf(const char *fmt, va_list args)
{
    int cnt = 0;
    for (int i = 0; fmt[i] != '\0'; i++)
    {
        if (fmt[i] == '%')
        {
            switch (fmt[++i])
            {
            case 'd':
            case 'i':
                // signed int
                cnt += writeSInt(va_arg(args, long));
                break;
            case 'u':
            case 'f':
                // unsigned int
                cnt += writeInt(va_arg(args, u64));
                break;
            case 'x':
            case 'X':
                // hex
                cnt += writeHexInt(va_arg(args, u64));
                break;
            case 'c':
                // char
                putChar(va_arg(args, int));
                cnt++;
                break;
            case 's':
                // string
                cnt += writeString(va_arg(args, char *));
                break;
            case 'p':
                // pointer address
                cnt += writeHexInt((u64)va_arg(args, void *));
                break;
            case '%':
                // escaped '%' sign
                putChar('%');
                cnt++;
                break;
            default:
                // invalid format - output '%' sign
                putChar('%');
                cnt++;
                break;
            }
        }
        else
        {
            putChar(fmt[i]);
            cnt++;
        }
    }
    return cnt;
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return vprintf(fmt, args);
}