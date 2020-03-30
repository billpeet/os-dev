#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

extern char readChar();

extern void putChar(char c);
extern int printf(const char *fmt, ...);
extern int vprintf(const char *fmt, va_list args);

#endif