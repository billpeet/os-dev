#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void setChar(const char c, int line, int pos);

void clearScreen();

void writeChar(char c);
void writeNewLine();
void printf(const char *fmt, ...);

#endif