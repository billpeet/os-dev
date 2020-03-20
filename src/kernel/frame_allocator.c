#include "frame_allocator.h"
#include "types.h"
#include "vga.h"
#include "boot_info.h"
#include "kernel.h"
#include <stdbool.h>

struct frame_header
{
    struct frame_header *next;
};

typedef struct frame_header frame_header_t;

u64 next_free_frame;
memory_map_entry_t *current_area;
frame_header_t *free_frames = NULLPTR;

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
            // writeStrHexInt("New area: ", entry->base_addr);
            current_area = entry;
            u64 start_frame = get_frame_containing_address(entry->base_addr);
            if (next_free_frame < start_frame)
                next_free_frame = start_frame;
            return;
        }
    }

    current_area = (void *)0;
}

void *allocate_frame()
{
    if (free_frames != NULLPTR)
    {
        void *free_frame = free_frames;
        // writeStrHexInt("Spare frame at: ", (u64)free_frame);
        free_frames = free_frames->next;
        return free_frame;
    }

    if ((u64)current_area == 0)
    {
        printf("Out of memory!\n");
        panic(3);
    }

    u64 current_area_last_frame = get_frame_containing_address(current_area->base_addr + current_area->length - 1);
    if (next_free_frame > current_area_last_frame)
    {
        // We've gone off the end of the current area, choose a new area and try again
        printf("Previous area full, grab new area\n");
        next_area();
        return allocate_frame();
    }
    else if ((next_free_frame >= kernel_start && next_free_frame <= kernel_end) ||
             (next_free_frame >= multiboot_start && next_free_frame <= multiboot_end))
    {
        // This frame belongs to the kernel or the multiboot header, jump forward a frame and try again
        printf("Hit the kernel/multiboot, grab new frame\n");
        next_free_frame++;
        return allocate_frame();
    }
    else
    {
        return (void *)(PAGE_SIZE * next_free_frame++);
    }
}

void deallocate_frame(void *frame)
{
    frame_header_t *header = (frame_header_t *)frame;
    header->next = NULLPTR;
    if (free_frames != NULLPTR)
        free_frames->next = header;

    free_frames = header;
}

void init_frame_allocator()
{
    next_free_frame = 1; // skip first frame - it makes detecting null pointers too hard :D
    next_area();
}