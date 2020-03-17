#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void setChar(const char c, int line, int pos);

void clearScreen();

void writeString(const char *str);
void writeChar(char c);
void writeInt(unsigned long i);
void writeHexInt(unsigned long i);
void writeSInt(long i);
void writeNewLine();

#endif