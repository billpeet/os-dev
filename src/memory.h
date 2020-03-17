#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096

struct page_table
{
    u64 entries[512];
};

struct page_frame
{
    u64 number;
};

typedef struct page_table page_table_t;
typedef struct page_frame page_frame_t;

page_table_t *level_4_table();
page_table_t *level_3_table();
page_table_t *level_2_table();
void init_paging();
void *get_physaddr(void *virt_addr);
u64 allocate_frame();
void init_frame_allocator();

#endif