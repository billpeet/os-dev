#include "idt.h"
#include <stdio.h>
#include "kernel.h"
#include "x86.h"
#include "gcc-attributes.h"
#include "drivers/keyboard.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define IDT_SIZE 256
#define TASK_GATE 0x85
#define TRAP_GATE 0x8f
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

struct idt_pointer
{
    u16 limit;
    u64 base;
} PACKED;

extern void *code_selector;

typedef struct IDT_entry
{
    u16 offset_lowerbits;
    u16 selector;
    u16 options;
    u16 offset_middlebits;
    u32 offset_higherbits;
    u32 reserved;
} PACKED IDT_entry_t;

IDT_entry_t idt[IDT_SIZE] ALIGNED(4);

void init_pic()
{
    // INIT PIC1 & PIC2
    // ICW1 - begin init
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2 - remap offset address of IDT
    // Must be beyond 0x20 because first 32 interrupts are reserved for CPU exceptions
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3 - no cascading
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4 - environment info (0x01 means 80x86 mode)
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask interrupts
    outb(0x21, 0xff);
    outb(0xA1, 0xff);
}

void handler_complete()
{
    outb(0x20, 0x20);
}

int_handler_t tm_handlers[100];

INTERRUPT void timer_handler(interrupt_frame_t *frame)
{
    cli();
    handler_complete();
    ticks++;
    sti();
}

int_handler_t kb_handlers[100];

bool shift;
bool caps;

INTERRUPT void keyboard_handler(interrupt_frame_t *frame)
{
    cli();
    handler_complete();
    keyboard_rec();
    sti();
}

INTERRUPT void breakpoint_handler(interrupt_frame_t *frame)
{
    u64 rax;
    asm volatile("mov %%rcx, %0"
                 : "=r"(rax)::"rcx");
    printf("Breakpoint! rcx: 0x%x\n", rax);
}

NORETURN INTERRUPT void double_fault_handler(interrupt_frame_t *frame, unsigned long long error_code)
{
    panic("Double fault! Error code %u\n", error_code);
}

INTERRUPT void page_fault_handler(interrupt_frame_t *frame, unsigned long long error_code)
{
    printf("Page fault!\n");
    printf("IP: 0x%x, SP: 0x%x\n", frame->rip);
    u64 cr2;
    asm volatile("mov %%cr2, %0"
                 : "=r"(cr2));
    if (error_code & 0b10)
        printf("Attempted to write 0x%x\n", cr2);
    else
        printf("Attempted to read 0x%x\n", cr2);

    if (error_code & 0b1)
        printf("Page protection violation\n");
    else
        printf("Non-present page\n");
    printf("\n");

    // We have to try and fix it, or else panic
    panic("Page fault");
}

NORETURN INTERRUPT void general_protection_fault_handler(interrupt_frame_t *frame, unsigned long long error_code)
{
    panic("General protection fault at IP 0x%x, SP 0x%x\nError code %u\n", frame->rip, frame->rsp, error_code);
}

int register_kbhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (kb_handlers[i].handler == NULL)
        {
            kb_handlers[i] = handler;
            return i;
        }
    }
    panic("All handlers are full!\n");
}

void unregister_kbhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (kb_handlers[i].handler == handler.handler && kb_handlers[i].task == handler.task)
        {
            kb_handlers[i].handler = NULL;
            kb_handlers[i].task = NULL;
            break;
        }
    }
}

int register_tmhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (tm_handlers[i].handler == NULL)
        {
            tm_handlers[i] = handler;
            return i;
        }
    }
    panic("All handlers are full!\n");
}

void unregister_tmhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (tm_handlers[i].handler == handler.handler && tm_handlers[i].task == handler.task)
        {
            tm_handlers[i].handler = NULL;
            tm_handlers[i].task = NULL;
            break;
        }
    }
}

void setup_idt_entry(struct IDT_entry *entry, size_t handler_address, u16 options)
{
    entry->offset_lowerbits = (u16)handler_address;
    entry->selector = 8;
    entry->options = options;
    entry->offset_middlebits = handler_address >> 16;
    entry->offset_higherbits = handler_address >> 32;
    entry->reserved = 0;
}

void init_interrupts()
{
    init_pic();

    u16 options_present = 0b1000111000000000; // Present bit set (15)
    u16 options_trap = 0b1000111100000000;    // Present bit (15) and trap bit (8) set

    setup_idt_entry(&idt[3], (size_t)breakpoint_handler, options_trap);                    // Breakpoint exception
    setup_idt_entry(&idt[8], (size_t)double_fault_handler, options_present);               // Double fault exception
    setup_idt_entry(&idt[13], (size_t)general_protection_fault_handler, options_present);  // General protection fault exception
    setup_idt_entry(&idt[14], (size_t)page_fault_handler, options_present);                // Page fault exception
    setup_idt_entry(&idt[TIMER_HANDLER_ID], (size_t)timer_handler, options_trap);          // Timer interrupt
    setup_idt_entry(&idt[KEYBOARD_HANDLER_ID], (size_t)keyboard_handler, options_present); // Keyboard interrupt

    // Load IDT
    lidt(idt, sizeof(idt));

    // // Unmask interrupts 0 & 1
    outb(0x21, 0b11111100);
    outb(0xA1, 0b11111111);

    // Enable interrupts
    sti();
}
