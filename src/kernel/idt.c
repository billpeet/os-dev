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
    asm("cli");
    // Save running task's current stack pointer
    task_t *previous_task = running_task;
    u64 rsp, rbp;
    asm volatile("mov %%rsp, %%rax\n\rmov %%rax, %0"
                 : "=m"(rsp)::"rax");
    asm volatile("mov %%rbp, %%rax\n\rmov %%rax, %0"
                 : "=m"(rbp)::"rax");
    handler_complete();

    // for (u32 i = 0; i < 100; i++)
    // {
    //     if (tm_handlers[i].handler)
    //     {
    //         tm_handlers[i].task->next = running_task;
    //         display_current_task();
    //         task_t new_task;
    //         create_task_with_stack(&new_task, &&back_again, previous_task->regs.flags, (u64 *)previous_task->regs.cr3, rbp, rsp - 8);

    //         task_t *old_next = tm_handlers[i].task->next;
    //         running_task = tm_handlers[i].task;
    //         running_task->next = &new_task;
    //         running_task->regs.rip = (u64)tm_handlers[i].handler;
    //         running_task->regs.rsp -= 8;
    //         switch_task(&previous_task->regs, &running_task->regs);
    //     back_again:
    //         tm_handlers[i].task->next = old_next;
    //     }
    // }

    static u64 i = 0;
    i++;

    // if (i % 40 == 0)
    // if (i == 10 || i == 20)
    // {
    //     // switch tasks
    //     task_t *next_task = running_task->next;
    //     // Don't switch if there's only 1 task or the next task is invalid
    //     if (next_task != running_task && next_task != NULLPTR)
    //     {
    //         frame->flags = next_task->regs.flags;
    //         frame->rip = (u64 *)next_task->regs.rip;
    //         printf("rip has been set to %p\n", frame->rip);
    //         frame->rsp = (u64 *)next_task->regs.rsp;
    //         running_task = running_task->next;
    //         printf("TM: Jumping to rsp %p, rip %p\n", frame->rip, frame->rsp);
    //         display_current_task();
    //     }
    // }

    asm("sti");
}

int_handler_t kb_handlers[100];

char last_char;

INTERRUPT void keyboard_handler(interrupt_frame_t *frame)
{
    asm("cli");
    printf("Frame addr %p pointer addr (%p) jumping to rsp %p, rip %p\n", frame, &frame, frame->rsp, frame->rip);
    // Save running task's current stack pointer
    task_t *previous_task = running_task;
    printf("previous task taskid=%u, ptr=%p ptrptr=%p\n", previous_task->id, previous_task, &previous_task);
    handler_complete();

    u16 status = inb(KEYBOARD_STATUS_PORT);

    if (status & 0x01)
    {

        char keycode = inb(KEYBOARD_DATA_PORT);

        if (keycode < 0)
            return;

        char c;

        if (keycode == ENTER_KEY_CODE)
            c = '\n';
        else
            c = keyboard_map[(u8)keycode];

        for (u32 i = 0; i < 100; i++)
        {
            if (kb_handlers[i].handler)
            {
                task_t *old_next = kb_handlers[i].task->next;
                kb_handlers[i].task->next = running_task;
                display_current_task();
                task_t new_task;

                running_task = kb_handlers[i].task;
                running_task->next = &new_task;
                running_task->regs.rip = (u64)kb_handlers[i].handler;
                printf("rip is now %x\n", running_task->regs.rip);
                running_task->regs.rsp -= 8;
                asm volatile("mov %0, %%rdi\n\t" ::"m"(c));
                last_char = c;

                u64 rsp, rbp;
                asm volatile("mov %%rsp, %%rax\n\rmov %%rax, %0"
                             : "=m"(rsp)::"rax");
                asm volatile("mov %%rbp, %%rax\n\rmov %%rax, %0"
                             : "=m"(rbp)::"rax");

                create_task_with_stack(&new_task, &&back_again, previous_task->regs.flags, (u64 *)previous_task->regs.cr3, rbp, rsp - 8);

                // printf("current rsp %x, previous task taskid=%u, ptr=%p ptrptr=%p, frame rip %p\n", rsp, previous_task->id, previous_task, &previous_task, frame->rip);
                // printf("frame ptr %p, value %p, rip ptr %p, value %p\n", &frame, frame, &frame->rip, frame->rip);
                printf("value at 0x44ffc8: 0x%x\n", *((u64 *)0x44ffc8));
                switch_task(&previous_task->regs, &running_task->regs);
            back_again:
                // asm volatile("pop %rax\n\t");
                // asm volatile("pop %rax\n\t");
                // printf("current rsp %x, previous task taskid=%u, ptr=%p ptrptr=%p, frame rip %p\n", rsp, previous_task->id, previous_task, &previous_task, &frame->rip);
                // printf("frame ptr %p, value %p, rip ptr %p, value %p\n", &frame, frame, &frame->rip, frame->rip);
                printf("value at 0x44ffc8: 0x%x\n", *((u64 *)0x44ffc8));
                // printf("back again\n");
                kb_handlers[i].task->next = old_next;
            }
        }
    }
    running_task = previous_task;
    display_current_task();
    printf("Frame addr %p pointer addr (%p) jumping to rsp %p, rip %p\n", frame, &frame, frame->rsp, frame->rip);
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
    printf("IP: 0x%x, SP: 0x%x\n", frame->rip);
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

INTERRUPT void general_protection_fault_handler(interrupt_frame_t *frame, unsigned long long error_code)
{
    printf("\nPANIC: general protection fault!\n");
    printf("Error code %u\n", error_code);
    printf("Press any key to continue...\n");
    asm("hlt");
}

INTERRUPT void random_handler(struct interrupt_frame *frame)
{
    printf("interrupt!\n");
}

int register_kbhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (kb_handlers[i].handler == NULLPTR)
        {
            kb_handlers[i] = handler;
            return i;
        }
    }
    printf("All handlers are full!\n");
}

void unregister_kbhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (kb_handlers[i].handler == handler.handler && kb_handlers[i].task == handler.task)
        {
            kb_handlers[i].handler = NULLPTR;
            kb_handlers[i].task = NULLPTR;
            break;
        }
    }
}

int register_tmhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (tm_handlers[i].handler == NULLPTR)
        {
            tm_handlers[i] = handler;
            return i;
        }
    }
    printf("All handlers are full!\n");
}

void unregister_tmhandler(int_handler_t handler)
{
    for (u32 i = 0; i < 100; i++)
    {
        if (tm_handlers[i].handler == handler.handler && tm_handlers[i].task == handler.task)
        {
            tm_handlers[i].handler = NULLPTR;
            tm_handlers[i].task = NULLPTR;
            break;
        }
    }
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

    setup_idt_entry(&idt[3], (u64)breakpoint_handler, options_trap);                   // Breakpoint exception
    setup_idt_entry(&idt[8], (u64)double_fault_handler, options_present);              // Double fault exception
    setup_idt_entry(&idt[13], (u64)general_protection_fault_handler, options_present); // General protection fault exception
    setup_idt_entry(&idt[14], (u64)page_fault_handler, options_present);               // Page fault exception
    setup_idt_entry(&idt[32], (u64)timer_handler, options_trap);                       // Timer interrupt
    setup_idt_entry(&idt[33], (u64)keyboard_handler, options_present);                 // Keyboard interrupt

    setup_idt_entry(&idt[49], (u64)random_handler, options_present);
    lidt();

    // // Unmask interrupts 0 & 1
    outb(0x21, 0b11111100);
    outb(0xA1, 0b11111111);
}
