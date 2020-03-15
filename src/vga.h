#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void setChar(const char c, int line, int pos);

void clearScreen();

void writeString(const char *str);
void writeChar(char c);
void writeInt(int i);

#endif