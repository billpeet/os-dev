#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MAX VGA_WIDTH * VGA_HEIGHT

// Moves all characters up one line
extern void vga_moveUp();

// Sets character at line/char position
extern void setChar(const char c, int line, int pos);

// Clears whole screen
extern void vga_clearScreen();

// Writes character at cursor position
extern void vga_writeChar(char c);

#endif