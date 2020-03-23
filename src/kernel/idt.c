#include "idt.h"
#include "keyboard_map.h"
#include "vga.h"
#include "kernel.h"
#include "x86.h"

#define IDT_SIZE 256
#define TASK_GATE 0x85
#define TRAP_GATE 0x8f
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#ifdef __GNUC__
#define INTERRUPT __attribute__((interrupt))
#define EXCEPTION __attribute__((exception))
#else
#define INTERRUPT
#endif

struct idt_pointer
{
    u16 limit;
    u64 base;
} __attribute((packed));

extern void load_idt(void);
extern void *code_selector;

struct IDT_entry
{
    u16 offset_lowerbits;
    u16 selector;
    u16 options;
    u16 offset_middlebits;
    u32 offset_higherbits;
    u32 reserved;
} __attribute((packed));

// struct IDT_entry IDT[IDT_SIZE] __attribute((aligned(4)));
extern struct IDT_entry idt[IDT_SIZE];

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

INTERRUPT void timer_handler(interrupt_frame_t *frame)
{
    asm("cli");
    handler_complete();
    asm("sti");
}

int_handler_t keyboardHandler;

INTERRUPT void keyboard_handler(interrupt_frame_t *frame)
{
    // Save running task's current stack pointer
    asm("cli");
    u64 rsp;
    asm volatile("mov %%rsp, %%rax\n\rmov %%rax, %0"
                 : "=m"(rsp)::"rax");
    printf("rsp: %x\n", rsp);

    printf("INTERRUPT: prev task %u, rip %p, rsp %p (prev rip %x, prev rsp %x)\n", running_task->id, frame->instruction_pointer, frame->stack_pointer, running_task->regs.rip, running_task->regs.rsp);

    printf("\n");
    handler_complete();

    u16 status = read_port(KEYBOARD_STATUS_PORT);

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

        if (keyboardHandler.handler)
        {
            if (keyboardHandler.task != running_task)
            {
                // Need to switch tasks
                keyboardHandler.task->next = running_task;
                // printf("Switching from %u to %u due to interrupt\n", running_task->id, keyboardHandler.task->id);
                setChar(running_task->id + 48, VGA_HEIGHT - 1, VGA_WIDTH - 1);
                u64 prev_rip = running_task->regs.rip;
                u64 prev_rsp = running_task->regs.rsp;
                task_t *previous_task = running_task;
                task_t new_task;
                create_task(&new_task, &&back_again, previous_task->regs.flags, (u64 *)previous_task->regs.cr3);
                new_task.regs.rsp = rsp;

                running_task = keyboardHandler.task;
                running_task->next = previous_task;
                // running_task->next = &new_task;
                running_task->regs.rip = (u64)keyboardHandler.handler;
                running_task->regs.rsp -= 8;
                previous_task->regs.rip = (u64) && back_again;
                asm volatile("mov %0, %%rdi\n\t" ::"m"(c));
                // switch_task(&previous_task->regs, &running_task->regs);
            back_again:
                printf("back again\n");
                printf("Current task: %u\n", running_task->id);

                // running_task->regs.rip = prev_rip;
                // running_task->regs.rsp = prev_rsp;
                // running_task->regs.rip = (u64)frame->instruction_pointer;
                // running_task->regs.rsp = (u64)frame->stack_pointer;
            }
            else
            {
                // Invoke handler on running task
                keyboardHandler.handler(c);
            }
        }
        else
            writeChar(c);
    }
    printf("exiting keyboard handler\n");
    asm("sti");
}

INTERRUPT void breakpoint_handler(interrupt_frame_t *frame)
{
    printf("Breakpoint!\n");
    u64 rax;
    asm volatile("mov %%rcx, %0"
                 : "=r"(rax)::"rcx");
    printf("rcx: %x\n", rax);
}

INTERRUPT void double_fault_handler(interrupt_frame_t *frame)
{
    printf("PANIC: double fault!\n");
    printf("\nPress any key to continue...\n");
    asm("hlt");
}

INTERRUPT void page_fault_handler(interrupt_frame_t *frame, unsigned long long error_code)
{
    printf("Page fault!\n");
    printf("IP: 0x%x, SP: 0x%x\n", frame->instruction_pointer);
    if (error_code & 0b1)
        printf("Page protection violation\n");
    else
        printf("Non-present page\n");
    if (error_code & 0b10)
        printf("Attempting to write\n");
    else
        printf("Attempting to read\n");
    printf("\n");
    // We have to try and fix it, or else just hang
    asm("hlt");
}

INTERRUPT void random_handler(struct interrupt_frame *frame)
{
    printf("interrupt!\n");
}

void register_handler(int_handler_t handler)
{
    keyboardHandler = handler;
}

void unregister_handler(int_handler_t handler)
{
    keyboardHandler.task = NULLPTR;
    keyboardHandler.handler = NULLPTR;
}

void setup_idt_entry(struct IDT_entry *entry, u64 handler_address, u16 options)
{
    entry->offset_lowerbits = handler_address & 0xffff;
    entry->selector = 8;
    entry->options = options;
    entry->offset_middlebits = (handler_address & 0xffff0000) >> 16;
    entry->offset_higherbits = (handler_address & 0xffffffff00000000) >> 32;
    entry->reserved = 0;
}

void init_interrupts()
{
    init_pic();

    u16 options_present = 0b1000111000000000; // Present bit set (15)
    u16 options_trap = 0b1000111100000000;    // Present bit (15) and trap bit (8) set

    setup_idt_entry(&idt[3], (u64)breakpoint_handler, options_trap);      // Breakpoint exception
    setup_idt_entry(&idt[8], (u64)double_fault_handler, options_present); // Double fault exception
    setup_idt_entry(&idt[14], (u64)page_fault_handler, options_present);  // Page fault exception
    setup_idt_entry(&idt[32], (u64)timer_handler, options_trap);          // Timer interrupt
    setup_idt_entry(&idt[33], (u64)keyboard_handler, options_present);    // Keyboard interrupt

    setup_idt_entry(&idt[49], (u64)random_handler, options_present);
    load_idt();

    // // Unmask interrupts 0 & 1
    write_port(0x21, 0b11111101);
    write_port(0xA1, 0b11111111);
}
