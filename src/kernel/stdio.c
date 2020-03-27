#include "stdio.h"
#include "vga.h"
#include <stdarg.h>
#include "types.h"
#include <string.h>
#include "serial.h"

#define HEXCHAR(c) c >= 10 ? c + 87 : c + 48

void writeChar(char c)
{
    vga_writeChar(c);
    write_serial(c);
}

int writeString(const char *str)
{
    int i = 0;
    for (i = 0; str[i] != '\0'; i++)
        writeChar(str[i]);
    return i - 1;
}

int writeInt(u64 i)
{
    if (i < 10)
    {
        writeChar(i + 48);
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
        writeChar('-');
        cnt++;
        i *= -1;
    }
    return cnt + writeInt(i);
}

int writeHexInt(u64 i)
{
    if (i < 16)
    {
        writeChar(HEXCHAR((char)i));
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

int printf(const char *fmt, ...)
{
    int cnt = 0;
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
                writeChar(va_arg(args, int));
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
                writeChar('%');
                cnt++;
                break;
            default:
                // invalid format - output '%' sign
                writeChar('%');
                cnt++;
                break;
            }
        }
        else
        {
            writeChar(fmt[i]);
            cnt++;
        }
    }
    return cnt;
}