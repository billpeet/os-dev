#include "alloc.h"
#include "frame_allocator.h"
#include "paging.h"
#include "types.h"
#include "stdio.h"
#include "kernel.h"
#include <stddef.h>

#define HEAP_SIZE 1000 * PAGE_SIZE // 1000 KiB
#define HEAP_START 0x444444440000
#define HEAP_END HEAP_START + HEAP_SIZE
void *current;

struct heap_header
{
    size_t size;
    struct heap_header *next;
};

typedef struct heap_header heap_header_t;

heap_header_t *free_head = NULL;

void init_heap()
{
    for (int i = 1; i < HEAP_SIZE; i += PAGE_SIZE)
    {
        size_t phys_frame = (size_t)allocate_frame();
        map_page(phys_frame, (u8 *)HEAP_START + i, 0b10);
    }
    current = (void *)HEAP_START;
}

void *malloc(size_t size)
{
    if (free_head != NULL)
    {
        if (free_head->size >= size)
        {
            heap_header_t *free_bit = free_head;
            free_head = free_bit->next;
            return (u8 *)free_bit + sizeof(heap_header_t);
        }
        else
        {
            heap_header_t *prev = free_head;
            while (prev->next != NULL)
            {
                heap_header_t *curr = prev->next;
                if (curr->size >= size)
                {
                    heap_header_t *free_bit = curr;
                    prev->next = curr->next;
                    return (u8 *)free_bit + sizeof(heap_header_t);
                }
                prev = curr;
            }
        }
    }

    heap_header_t *header = (heap_header_t *)current;
    header->size = size;
    current = (u8 *)header + sizeof(heap_header_t) + size;
    if ((size_t)current >= HEAP_END)
        panic("Heap allocation failed: heap is full!\n");
    return (u8 *)header + sizeof(heap_header_t);
}

void free(void *ptr)
{
    heap_header_t *header = (heap_header_t *)((u8 *)ptr - sizeof(heap_header_t));
    header->next = free_head;
    free_head = header;
}