#include "boot_info.h"
#include <stdio.h>

size_t kernel_start;
size_t kernel_end;
size_t multiboot_start;
size_t multiboot_end;

size_t kernel_page_table_start;
size_t kernel_page_table_end;

memory_map_entry_t *memory_map;
size_t memory_map_count;

void init_boot_info(boot_info_t *boot_info)
{
    // printf("Total size: %u\n", boot_info->total_size);
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;"
                 : "=m"(kernel_page_table_start)::"rax");
    kernel_page_table_end = kernel_page_table_start + 4096 * 3;

    boot_tag_t *tag = (boot_tag_t *)boot_info + 1;
    for (int i = 0; i < 100; i++)
    {
        // printf("Type: %u, size: %u\n", tag->type, tag->size);

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
            for (size_t i = 0; i < memory_map_count; i++)
            {
                memory_map_entry_t *entry = memory_map + i;
                if (entry->type == 1)
                {
                    // printf("  start: %x, length: %x\n", entry->base_addr, entry->length);
                }
            }
        }
        else if (tag->type == 9)
        {
            // ELF symbols
            // printf("ELF symbol - size: %u\n", tag->size);
            u32 *ptr = (u32 *)(tag + 1);
            ptr += 3;
            elf_section_entry_t *entry = (elf_section_entry_t *)ptr;
            // printf("Kernel sections: \n");
            size_t min = 0xffffffffffffffff;
            size_t max = 0x0;
            while ((size_t)entry < (size_t)tag + tag->size)
            {
                if (entry->type != 0) // skip unused (type 0x0)
                {
                    if (entry->address < min)
                        min = entry->address;
                    if (entry->address + entry->size > max)
                        max = entry->address + entry->size;
                    // printf("  addr: %x, size: %x, flags: %x\n", entry->address, entry->size, entry->flags);
                }
                entry++;
            }
            // printf("kernel start: %x, kernel end: %x\n", min, max);
            // printf("boot_info start: %p, boot_info end: %p\n", boot_info, boot_info + boot_info->total_size);

            kernel_start = min;
            kernel_end = max;
            multiboot_start = (size_t)boot_info;
            multiboot_end = (size_t)boot_info + boot_info->total_size;
        }

        tag = (boot_tag_t *)((u8 *)tag + ((tag->size + 7) & ~7));
    }
}