#include "idt.h"
#include "keyboard_map.h"
#include "vga.h"

#define IDT_SIZE 256
#define TASK_GATE 0x85
#define TRAP_GATE 0x8f
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#ifdef __GNUC__
#define INTERRUPT __attribute__((interrupt))
#else
#define INTERRUPT
#endif

extern void load_idt(unsigned long *idt_ptr);

struct interrupt_frame;

struct IDT_entry
{
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void init_pic()
{
    // INIT PIC1 & PIC2
    // ICW1 - begin init
    write_port(0x20, 0x11);
    write_port(0xA0, 0x11);

    // ICW2 - remap offset address of IDT
    // Must be beyond 0x20 because first 32 interrupts are reserved for CPU exceptions
    write_port(0x21, 0x20);
    write_port(0xA1, 0x28);

    // ICW3 - no cascading
    write_port(0x21, 0x04);
    write_port(0xA1, 0x02);

    // ICW4 - environment info (0x01 means 80x86 mode)
    write_port(0x21, 0x01);
    write_port(0xA1, 0x01);

    // Mask interrupts
    write_port(0x21, 0xff);
    write_port(0xA1, 0xff);
}

void handler_complete()
{
    write_port(0x20, 0x20);
}

INTERRUPT void timer_handler(struct interrupt_frame *frame)
{
    handler_complete();
}

keyboardHandlerFn keyboardHandler;

INTERRUPT void keyboard_handler(struct interrupt_frame *frame)
{
    handler_complete();

    unsigned char status = read_port(KEYBOARD_STATUS_PORT);
    if (status & 0x01)
    {

        char keycode = read_port(KEYBOARD_DATA_PORT);
        if (keycode < 0)
            return;

        char c;

        if (keycode == ENTER_KEY_CODE)
            c = '\n';
        else
            c = keyboard_map[(unsigned char)keycode];

        if (keyboardHandler)
            keyboardHandler(c);
        else
            writeChar(c);
    }
}

INTERRUPT void breakpoint_handler(struct interrupt_frame *frame)
{
    writeString("Breakpoint!\n");
}

INTERRUPT void double_fault_handler(struct interrupt_frame *frame, unsigned int error_code)
{
    writeString("PANIC: double fault!\n");
    writeString("Error code ");
    writeChar(error_code + 48);
    writeString("\nPress any key to continue...\n");
    asm("hlt");
}

void register_handler(keyboardHandlerFn handler)
{
    keyboardHandler = handler;
}

void unregister_handler(keyboardHandlerFn handler)
{
    keyboardHandler = 0;
}

void setup_idt_entry(struct IDT_entry *entry, unsigned long handler_address, unsigned char type_attr)
{
    entry->offset_lowerbits = handler_address & 0xffff;
    entry->selector = KERNEL_CODE_SEGMENT_OFFSET;
    entry->zero = 0;
    entry->type_attr = type_attr;
    entry->offset_higherbits = (handler_address & 0xffff0000) >> 16;
}

void init_interrupts()
{
    init_pic();

    setup_idt_entry(&IDT[0x03], (unsigned long)breakpoint_handler, INTERRUPT_GATE);   // Breakpoint exception
    setup_idt_entry(&IDT[0x08], (unsigned long)double_fault_handler, INTERRUPT_GATE); // Double fault exception
    setup_idt_entry(&IDT[0x20], (unsigned long)timer_handler, INTERRUPT_GATE);        // Timer interrupt
    setup_idt_entry(&IDT[0x21], (unsigned long)keyboard_handler, INTERRUPT_GATE);     // Keyboard interrupt

    // Fill IDT descriptor
    unsigned long idt_address = (unsigned long)IDT;
    unsigned long idt_ptr[2];
    idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;

    load_idt(idt_ptr);

    // Unmask interrupts 0 & 1
    write_port(0x21, 0b11111100);
    write_port(0xA1, 0b11111111);
}
