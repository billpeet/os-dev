#ifndef VGA_H
#define VGA_H

void setChar(const char c, int line, int pos);

void clearScreen();

void writeString(const char *str);
void writeChar(char c);

#endif