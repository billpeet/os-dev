#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MAX VGA_WIDTH *VGA_HEIGHT

extern void vga_moveUp();

extern void setChar(const char c, int line, int pos);

extern void vga_clearScreen();

extern void vga_writeChar(char c);

#endif