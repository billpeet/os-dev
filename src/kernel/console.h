#ifndef CONSOLE_H
#define CONSOLE_H

extern void init_console();
extern void queue(char c);
extern int char_available();
extern char dequeue();

#endif