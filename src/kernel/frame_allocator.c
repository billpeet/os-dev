#include "frame_allocator.h"
#include "types.h"
#include "stdio.h"
#include "boot_info.h"
#include "kernel.h"
#include <stdbool.h>
#include <stddef.h>

struct frame_header
{
    struct frame_header *next;
};

typedef struct frame_header frame_header_t;

size_t next_free_frame;
memory_map_entry_t *current_area;
frame_header_t *free_frames = NULL;

size_t get_frame_containing_address(size_t addr)
{
    return addr / PAGE_SIZE;
}

void next_area()
{
    for (size_t i = 0; i < memory_map_count; i++)
    {
        memory_map_entry_t *entry = memory_map + i;
        if (entry->type == 1 && get_frame_containing_address(entry->base_addr + entry->length - 1) >= next_free_frame)
        {
            current_area = entry;
            size_t start_frame = get_frame_containing_address(entry->base_addr);
            if (next_free_frame < start_frame)
                next_free_frame = start_frame;
            return;
        }
    }
    current_area = NULL;
}

void *allocate_frame()
{
    if (free_frames != NULL)
    {
        void *free_frame = free_frames;
        free_frames = free_frames->next;
        return free_frame;
    }

    if (current_area == NULL)
        panic("Out of memory!\n");

    size_t current_area_last_frame = get_frame_containing_address(current_area->base_addr + current_area->length - 1);
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
    header->next = NULL;
    if (free_frames != NULL)
        free_frames->next = header;

    free_frames = header;
}

void init_frame_allocator()
{
    next_free_frame = 1; // skip first frame - it makes detecting null pointers too hard :D
    next_area();
}