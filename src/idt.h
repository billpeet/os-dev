#ifndef IDT_H
#define IDT_H

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned short data);

void init_interrupts(void);

typedef void (*handlerFn)(void);

void register_handler(handlerFn handler);
void unregister_handler(handlerFn handler);

#endif