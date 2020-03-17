#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define ENTRY_COUNT 512

struct page_table
{
    u64 entries[ENTRY_COUNT];
};

typedef struct page_table page_table_t;

page_table_t *level_4_table();

u64 get_physaddr(void *virt_addr);

void map_page(u64 phys_addr, void *virt_addr, u8 flags);
void unmap_page(void *virt_addr);

#endif