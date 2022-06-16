#include <stdio.h>
#include <stdio.h>
#include "console.h"
#include "drivers/vga.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "kernel.h"
#include "drivers/keyboard.h"
#include <stdint.h>
#include "serial.h"
#include "task.h"
#include "idt.h"
#include "fat.h"

#define HEXCHAR(c) c >= 10 ? c + 87 : c + 48
#define WHITESPACE " \t\n\r\f\v"
#define isws(c) (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v')

FILE stds[3];

void init_stdio()
{
    stds[0]._flag = 0;
    stds[1]._flag = 0;
    stds[2]._flag = 0;
}

int getc(FILE *stream)
{
    switch (stream->_flag)
    {
    case 0:
        // Console
        while (char_available() == 0)
            wait_for_interrupt(KEYBOARD_HANDLER_ID);
        return dequeue();
        break;
    case 1:
        // String
        stream->_cnt++;
        return *stream->_ptr++;
        break;
    default:
        panic("Invalid FILE type");
    }
}

char *fgets(char *str, int count, FILE *stream)
{
    char *tmp = str;
    char c;
    while (count-- && (*tmp++ = c = getc(stream)) != EOF && c != '\n')
        ;
    if (count > 0)
        // Chop off EOF or newline character
        *(--tmp) = '\0';
    return str;
}

char *gets(char *str)
{
    char *tmp = str;
    char c;
    while ((*tmp++ = c = getchar()) != EOF && c != '\n')
        ;
    *(--tmp) = '\0';
    return str;
}

int vfscanf(FILE *stream, const char *fmt, va_list args)
{
    char c, d;
    int count;
    while ((c = *fmt++) != '\0')
    {
        if (isws(c))
            continue;
        if (c == '%')
        {
            // get by format
            switch (*(fmt++))
            {
            case 's':
                do
                {
                    d = fgetc(stream);
                } while (isws(d)); // absorb all trailing whitespace chars
                char *dest = va_arg(args, char *);
                int i;
                for (i = 0; d != EOF && isws(d) == 0; d = fgetc(stream), i++)
                    *dest++ = d;
                *dest = '\0';
                count += i;
                break;
            case 'i':
                // signed int
                do
                {
                    d = fgetc(stream);
                } while (isws(d)); // absorb all trailing whitespace chars
                long l = va_arg(args, long);
                for (i = 0; d != EOF && isws(d) == 0; d = fgetc(stream), i++)
                {
                }
                break;
            default:
                printf("invalid format\n");
                break;
            }
        }
        else
        {
            // specified character requested
            do
            {
                d = fgetc(stream);
            } while (isws(d));
            if (d != c)
                break;
        }
    }
    return count;
}

int fscanf(FILE *stream, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vfscanf(stream, fmt, args);
    va_end(args);
    return res;
}

int scanf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vfscanf(stdin, fmt, args);
    va_end(args);
    return res;
}

int sscanf(char *str, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    FILE str_file;
    str_file._flag = 1;             // string type
    str_file._base = str;           // base ptr
    str_file._ptr = str_file._base; // set ptr at start
    str_file._cnt = 0;              // zero count
    int res = vfscanf(&str_file, fmt, args);
    va_end(args);
    return res;
}

int putc(int c, FILE *stream)
{
    switch (stream->_flag)
    {
    case 0:
        // Console
        write_serial(c);
        vga_writeChar(c);
        break;
    case 1:
        // String - advance count/ptr and return result
        *stream->_ptr++ = (char)c;
        stream->_cnt++;
        break;
    default:
        panic("Invalid FILE type");
    }
}

int fputs(const char *str, FILE *stream)
{
    int i = 0;
    for (i = 0; str[i] != '\0'; i++)
        putc(str[i], stream);
    return i - 1;
}

int writeInt(uint64_t i, uint8_t base, FILE *stream)
{
    if (i < base)
    {
        putc(HEXCHAR((char)i), stream);
        return 1;
    }
    else
    {
        char str[100];

        int j = 0;
        while (i > 0)
        {
            str[j] = HEXCHAR((char)(i % base));
            i /= base;
            j++;
        }
        str[j] = '\0';
        strrev(str);
        return fputs(str, stream);
    }
}

int writeSInt(long i, uint8_t base, FILE *stream)
{
    int cnt = 0;
    if (i < 0)
    {
        putc('-', stream);
        cnt++;
        i *= -1;
    }
    return cnt + writeInt(i, base, stream);
}

int vfprintf(FILE *stream, const char *fmt, va_list args)
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
                cnt += writeSInt(va_arg(args, long), 10, stream);
                break;
            case 'u':
            case 'f':
                // unsigned int
                cnt += writeInt(va_arg(args, uint64_t), 10, stream);
                break;
            case 'x':
            case 'X':
                // hex
                cnt += writeInt(va_arg(args, uint64_t), 16, stream);
                break;
            case 'c':
                // char
                putc(va_arg(args, int), stream);
                cnt++;
                break;
            case 's':
                // string
                cnt += fputs(va_arg(args, char *), stream);
                break;
            case 'p':
                // pointer address
                cnt += writeInt((size_t)va_arg(args, void *), 16, stream);
                break;
            case '%':
                // escaped '%' sign
                putc('%', stream);
                cnt++;
                break;
            default:
                // invalid format - output '%' sign
                putc('%', stream);
                cnt++;
                break;
            }
        }
        else
        {
            putc(fmt[i], stream);
            cnt++;
        }
    }
    return cnt;
}

int vsprintf(char *str, const char *fmt, va_list args)
{
    FILE str_file;
    str_file._flag = 1;             // string type
    str_file._base = str;           // base ptr
    str_file._ptr = str_file._base; // set ptr at start
    str_file._cnt = 0;              // zero count
    return vfprintf(&str_file, fmt, args);
}

int fprintf(FILE *stream, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vfprintf(stream, fmt, args);
    va_end(args);
    return res;
}

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vprintf(fmt, args);
    va_end(args);
    return res;
}

int sprintf(char *str, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vsprintf(str, fmt, args);
    va_end(args);
    return res;
}

void perror(const char *str)
{
    fputs(str, stderr);
    fputs(": ", stderr);
    // print error message?
}

#define BUFFER_SIZE 100

FILE *fopen(const char *filename, const char *mode)
{
    // get FAT
    // read_directory(0, );
}

int fclose(FILE *file)
{
    return 0;
}