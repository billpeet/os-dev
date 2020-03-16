#include "kernel.h"
#include "vga.h"
#include "idt.h"
#include "pong.h"
#include "shell.h"
#include "memory.h"
#include "types.h"

extern u16 code_selector;

void hlt()
{
    asm("hlt");
}

struct boot_info
{
    u32 total_size;
    u32 _reserved;
};

struct boot_tag
{
    u32 type;
    u32 size;
};

struct memory_map_entry
{
    u64 base_addr;
    u64 length;
    u32 type;
    u32 reserved;
};

struct elf_section_entry
{
    u32 name;
    u32 type;
    u64 flags;
    u64 address;
    u64 file_offset;
    u64 size;
    u32 link;
    u32 info;
    u64 addr_align;
    u64 entry_size;
};

typedef struct boot_info boot_info_t;
typedef struct boot_tag boot_tag_t;
typedef struct memory_map_entry memory_map_entry_t;
typedef struct elf_section_entry elf_section_entry_t;

void kmain(boot_info_t *boot_info)
{
    clearScreen();
    writeString("Welcome to PeetOS\n");
    writeString("Total size: ");
    writeInt(boot_info->total_size);
    writeString("\n");
    boot_tag_t *tag = (boot_tag_t *)boot_info + 1;
    for (int i = 0; i < 10; i++)
    {
        writeString("Type: ");
        writeInt(tag->type);
        writeString("\n");
        writeString("Size: ");
        writeInt(tag->size);
        writeString("\n");

        if (tag->type == 0)
            break;

        if (tag->type == 6)
        {
            // Memory map
            u32 *ptr = (u32 *)tag;
            ptr++; // Jump over tag header
            ptr++; // Jump over entry_size and entry_version
            ptr++;
            ptr++;
            memory_map_entry_t *entry = (memory_map_entry_t *)ptr;
            while (entry < tag + (tag->size / 8))
            {
                if (entry->type == 1)
                {
                    writeString("Base addr: ");
                    writeInt(entry->base_addr);
                    writeString("\nLength: ");
                    writeInt(entry->length);
                    writeString("\nType: ");
                    writeInt(entry->type);
                    writeString("\n");
                }
                entry++;
            }
        }
        else if (tag->type == 9)
        {
            // ELF symbols
            u32 *ptr = (u32 *)tag;
            ptr += 4;
            elf_section_entry_t *entry = (elf_section_entry_t *)ptr;
            writeString("Name: ");
            writeInt(entry->name);
        }

        u8 *ptr = (u8 *)tag;
        ptr += tag->size;
        ptr += (8 - (u64)ptr % 8);
        tag = (boot_tag_t *)ptr;
    }

    init_interrupts();
    // init_paging();

    shell();
    while (1)
        hlt();
}

void reboot()
{
    u8 temp;

    asm volatile("cli");
    do
    {
        temp = read_port(KEYBOARD_STATUS_PORT);
        if (temp & 0b10)
            read_port(KEYBOARD_DATA_PORT);
    } while (temp & 1);

    write_port(KEYBOARD_STATUS_PORT, 0xFE);
loop:
    asm volatile("hlt");
    goto loop;
}