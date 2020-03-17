#include "memory.h"
#include "types.h"
#include "vga.h"
#include "boot_info.h"

u64 next_free_frame;
memory_map_entry_t *current_area;

void flush_tlb()
{
    asm volatile(
        "mov %cr3, %rax\n\t"
        "mov %rax, %cr3\n\t");
}

u64 get_frame_containing_address(u64 addr)
{
    return addr / PAGE_SIZE;
}

page_table_t *level_4_table()
{
    u64 cr3;
    asm volatile(
        "mov %%cr3, %%rax\n\t"
        "mov %%rax, %0\n\t"
        : "=m"(cr3)
        :
        : "rax");
    return (page_table_t *)cr3;
}

void *get_page_table_entry(page_table_t *page_table, u64 id, u64 *addr, u8 *flags)
{
    u64 entry = page_table->entries[id];
    *addr = (entry >> 8) << 8;
    *flags = entry & 0xFF;
}

page_table_t *get_page_table(page_table_t *page_table, u64 id)
{
    return (page_table_t *)((page_table->entries[id] >> 8) << 8);
}

void *get_physaddr(void *virt_addr)
{
    u64 p4_index = ((u64)virt_addr >> 12 >> 9 >> 9 >> 9) & 0777;
    u64 p3_index = ((u64)virt_addr >> 12 >> 9 >> 9) & 0777;
    u64 p2_index = ((u64)virt_addr >> 12 >> 9) & 0777;
    u64 p1_index = ((u64)virt_addr >> 12) & 0777;

    writeString("Transforming virt_addr ");
    writeInt((u64)virt_addr);
    writeString(": P4 index ");
    writeInt((u64)p4_index);
    writeString(", P3 index ");
    writeInt((u64)p3_index);
    writeString(", P2 index ");
    writeInt((u64)p2_index);
    writeString(", P1 index ");
    writeInt((u64)p1_index);
    writeString("\n");

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = get_page_table(p4, p4_index);
    page_table_t *p2 = get_page_table(p3, p3_index);

    u64 addr;
    u8 flags;
    get_page_table_entry(p2, p2_index, &addr, &flags);
    if (flags & 0x80)
        // 4MiB page - return this
        return (void *)addr;

    page_table_t *p1 = (page_table_t *)addr;
    get_page_table_entry(p1, p1_index, &addr, &flags);
    return (void *)addr;
}

void map_page(void *phys_addr, void *virt_addr, unsigned int flags)
{
    u64 p4_index = ((u64)virt_addr >> 12 >> 9 >> 9 >> 9) & 0777;
    u64 p3_index = ((u64)virt_addr >> 12 >> 9 >> 9) & 0777;
    u64 p2_index = ((u64)virt_addr >> 12 >> 9) & 0777;
    u64 p1_index = ((u64)virt_addr >> 12) & 0777;

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = get_page_table(p4, p4_index);
    page_table_t *p2 = get_page_table(p3, p3_index);
    if (flags & 0x80)
    {
        // Creating a 'huge' 4MiB page
        writeString("Mapping a huge page at P2 index ");
        writeInt(p2_index);
        writeNewLine();

        p2->entries[p2_index] = (u64)phys_addr | (flags & 0xFF) | 0x01;
    }
    else
    {
        writeString("Mapping a standard page at P1 index ");
        writeInt(p1_index);
        writeNewLine();

        page_table_t *p1 = get_page_table(p2, p2_index);
        p1->entries[p1_index] = (u64)phys_addr | (flags & 0xFF) | 0x01;
    }

    flush_tlb();
}

void next_area()
{
    for (u64 i = 0; i < memory_map_count; i++)
    {
        memory_map_entry_t *entry = memory_map + i;
        if (entry->type == 1 && get_frame_containing_address(entry->base_addr + entry->length - 1) >= next_free_frame)
        {
            writeString("New area: ");
            writeHexInt(entry->base_addr);
            writeNewLine();
            current_area = entry;
            u64 start_frame = get_frame_containing_address(entry->base_addr);
            if (next_free_frame < start_frame)
                next_free_frame = start_frame;
            return;
        }
    }

    current_area = (void *)0;
}

u64 allocate_frame()
{
    if ((u64)current_area == 0)
        return 0;

    u64 current_area_last_frame = get_frame_containing_address(current_area->base_addr + current_area->length - 1);
    if (next_free_frame > current_area_last_frame)
    {
        // We've gone off the end of the current area, choose a new area and try again
        writeString("Previous area full, grab new area\n");
        next_area();
        return allocate_frame();
    }
    else if ((next_free_frame >= kernel_start && next_free_frame <= kernel_end) ||
             (next_free_frame >= multiboot_start && next_free_frame <= multiboot_end))
    {
        // This frame belongs to the kernel or the multiboot header, jump forward a frame and try again
        writeString("Hit the kernel/multiboot, grab new frame\n");
        next_free_frame++;
        return allocate_frame();
    }
    else
    {
        return next_free_frame++;
    }
}

void init_frame_allocator()
{
    next_free_frame = 0;
    next_area();
}