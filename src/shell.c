#include "shell.h"
#include "idt.h"
#include "vga.h"
#include "pong.h"
#include "alloc.h"
#include "string.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kernel.h"

char str[100];
int pos;

void shell_char(char c);

int starts_with(char *input, char const *check)
{
    char c;
    for (int i = 0; c = check[i]; i++)
    {
        if (!input[i] || input[i] != check[i])
            return 0;
    }
    return 1;
}

void shell_line_init()
{
    writeString("> ");
    for (int i = 0; i < 100; i++)
    {
        str[i] = '\0';
    }
    pos = 0;
}

void shell_execute()
{
    writeChar('\n');
    if (starts_with(str, "pong"))
    {
        writeString("Starting pong:\n");
        pong();
        writeString("done with pong\n");
        register_handler(shell_char);
    }
    else if (starts_with(str, "clear"))
    {
        clearScreen();
    }
    else if (starts_with(str, "newframe"))
    {
        void *new_frame = allocate_frame();
        writeString("New frame: ");
        writeHexInt((u64)new_frame);
        writeNewLine();
        // for (u64 i = 0;; i++)
        // {
        //     if (allocate_frame() == 0 && i != 0)
        //     {
        //         writeString("Allocated ");
        //         writeInt(i);
        //         writeString(" frames\n");
        //         break;
        //     }
        // }
    }
    else if (starts_with(str, "deaframe"))
    {
        void *new_frame1 = allocate_frame();
        void *new_frame2 = allocate_frame();
        void *new_frame3 = allocate_frame();
        writeString("New frame: ");
        writeHexInt((u64)new_frame3);
        writeString("\nNow deallocating again\n");
        deallocate_frame(new_frame3);
        deallocate_frame(new_frame2);
        deallocate_frame(new_frame1);
        void *new_new_frame = allocate_frame();
        writeString("Next frame: ");
        writeHexInt((u64)new_new_frame);
        writeNewLine();
    }
    else if (starts_with(str, "paging"))
    {
        u64 ptr = 0x00000fffffeee010;
        ptr = 0x20003;
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
    }
    else if (starts_with(str, "recursive"))
    {
        u64 l4_table_addr = (u64)level_4_table();
        page_table_t *p4 = (page_table_t *)0xfffffffffffff000;
        u64 phys_addr = get_physaddr(p4);
        writeString("P4 table:");
        writeHexInt((u64)p4);
        writeString(", phys_addr:");
        writeHexInt((u64)phys_addr);
        writeString(", actual:");
        writeHexInt((u64)l4_table_addr);
        writeNewLine();
        writeString("Attempting to deallocate: ");
        writeHexInt(p4->entries[0]);
        writeNewLine();
    }
    else if (starts_with(str, "newpage"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
    }
    else if (starts_with(str, "newhuge"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10000010); // huge
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
    }
    else if (starts_with(str, "unmap"))
    {
        u64 ptr = 0x1;
        ptr = 0x40000000; // P4 = 0, P3 = 1, P2 = 0, P1 = 0
        writeString("Mapping new page\n");
        map_page(0x5000, (void *)ptr, 0b10);
        writeString("Getting phys addr\n");
        u64 phys_addr = get_physaddr((void *)ptr);
        writeString("Phys_addr:");
        writeHexInt((u64)phys_addr);
        writeNewLine();
        int *int_ptr = (int *)ptr;
        writeString("Here's its value:");
        writeInt(*int_ptr);
        unmap_page((void *)ptr);
        writeString("\nUnampped, now attempting to get physical addr:\n");
        get_physaddr((void *)ptr);
        writeInt(*int_ptr);
    }
    else if (starts_with(str, "alloc"))
    {
        u32 *a = malloc(sizeof(u32));
        *a = 23;
        writeString("Allocated a u32 at ");
        writeInt((u64)a);
        writeString(", with value ");
        writeInt(*a);
        writeString("\n");
    }
    else if (starts_with(str, "breakpoint"))
    {
        writeString("Triggering breakpoint exception:\n");
        asm("int3");
    }
    else if (starts_with(str, "page fault"))
    {
        writeString("Triggering page fault exception:\n");
        int *ptr = (int *)0xdeadbeef;
        *ptr = 1;
    }
    else if (starts_with(str, "divide by zero"))
    {
        writeString("Triggering divide by zero exception:\n");
        int i = 0;
        i /= i;
    }
    else if (starts_with(str, "reboot"))
    {
        writeString("Rebooting...\n");
        reboot();
    }
    else
    {
        writeString("Unrecognised command '");
        char c;
        for (int i = 0; c = str[i] != '\0'; i++)
            writeChar(str[i]);
        writeString("'\n");
    }
    shell_line_init();
}

void shell_char(char c)
{
    if (c == '\b')
    {
        if (pos > 0)
        {
            str[--pos] = '\0';
            writeChar(c);
        }
    }
    else if (c == '\n')
        shell_execute();
    else
    {
        writeChar(c);
        str[pos++] = c;
    }
}

void shell(void)
{
    shell_line_init();
    register_handler(shell_char);

    while (1)
        asm("hlt");
}