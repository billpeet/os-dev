#include "boot_info.h"
#include "vga.h"

u64 kernel_start;
u64 kernel_end;
u64 multiboot_start;
u64 multiboot_end;
memory_map_entry_t *memory_map;
u64 memory_map_count;

void init_boot_info(boot_info_t *boot_info)
{
    writeString("Total size: ");
    writeInt(boot_info->total_size);
    writeString("\n");

    boot_tag_t *tag = (boot_tag_t *)boot_info + 1;
    for (int i = 0; i < 100; i++)
    {
        // writeString("Type: ");
        // writeInt(tag->type);
        // writeString(", Size: ");
        // writeInt(tag->size);
        // writeString("\n");

        if (tag->type == 0)
            break;

        if (tag->type == 6)
        {
            // Memory map
            u8 *ptr = (u8 *)tag;
            ptr += 8; // Jump over tag header
            ptr += 8; // Jump over entry_size and entry_version
            memory_map = (memory_map_entry_t *)ptr;
            memory_map_count = (tag->size - 16) / sizeof(memory_map_entry_t);
            for (u64 i = 0; i < memory_map_count; i++)
            {
                memory_map_entry_t *entry = memory_map + i;
                if (entry->type == 1)
                {
                    writeString("  start: ");
                    writeHexInt(entry->base_addr);
                    writeString(", length: ");
                    writeHexInt(entry->length);
                    writeString("\n");
                }
            }
        }
        else if (tag->type == 9)
        {
            // ELF symbols
            writeString("ELF symbol - size: ");
            writeInt(tag->size);
            writeNewLine();
            u32 *ptr = (u32 *)(tag + 1);
            ptr += 3;
            elf_section_entry_t *entry = (elf_section_entry_t *)ptr;
            writeString("Kernel sections: \n");
            u64 min = 0xffffffffffffffff;
            u64 max = 0x0;
            while ((u64)entry < (u64)tag + tag->size)
            {
                if (entry->type != 0) // skip unused (type 0x0)
                {
                    if (entry->address < min)
                        min = entry->address;
                    if (entry->address + entry->size > max)
                        max = entry->address + entry->size;
                    writeString("  addr: ");
                    writeHexInt(entry->address);
                    writeString(", size: ");
                    writeHexInt(entry->size);
                    writeString(", flags: ");
                    writeHexInt(entry->flags);
                    writeString("\n");
                }
                entry++;
            }
            writeString("kernel start: ");
            writeHexInt(min);
            writeString(", kernel end: ");
            writeHexInt(max);
            writeString("\n");

            writeString("boot info: ");
            writeHexInt((u64)boot_info);
            writeString(", kernel end: ");
            writeHexInt((u64)boot_info + boot_info->total_size);
            writeString("\n");

            kernel_start = min;
            kernel_end = max;
            multiboot_start = (u64)boot_info;
            multiboot_end = (u64)boot_info + boot_info->total_size;
        }

        tag = (boot_tag_t *)((u8 *)tag + ((tag->size + 7) & ~7));
    }
}