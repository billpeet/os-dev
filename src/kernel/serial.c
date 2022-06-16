#include "serial.h"
#include "x86.h"
#include "task.h"

#define PORT 0x3f8

void init_serial(void)
{
    outb(PORT + 1, 0x00); // Disable interrupts???
    outb(PORT + 3, 0x80); // Enable DLAB
    outb(PORT + 0, 0x03); // Set dvision to 3 (low byte) 38400 baud (high byte)
    outb(PORT + 1, 0x03); // 38400 baud (high byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xc7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0b); // IRQs enabled, RTS/DSR set
}

uint8_t serial_received()
{
    return inb(PORT + 5) & 0b1;
}

uint8_t read_serial()
{
    while (serial_received() == 0)
        yield();

    return inb(PORT);
}

uint8_t transmit_empty()
{
    return inb(PORT + 5) & 0x20;
}

void write_serial(uint8_t data)
{
    while (transmit_empty() == 0)
        yield();
    outb(PORT, data);
}