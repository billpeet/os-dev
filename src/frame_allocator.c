#include "frame_allocator.h"
#include "types.h"
#include "vga.h"
#include "boot_info.h"

u64 next_free_frame;
memory_map_entry_t *current_area;

u64 get_frame_containing_address(u64 addr)
{
    return addr / PAGE_SIZE;
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
        return PAGE_SIZE * next_free_frame++;
    }
}

void init_frame_allocator()
{
    next_free_frame = 0;
    next_area();
}