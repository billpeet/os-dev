#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include <stddef.h>

#define ENTRY_COUNT 512

typedef struct page_table
{
    u64 entries[ENTRY_COUNT];
} page_table_t;

page_table_t *level_4_table();

size_t get_physaddr(void *virt_addr);

void identity_map(void *virt_addr, u8 flags);
void map_page(size_t phys_addr, void *virt_addr, u8 flags);
void unmap_page(void *virt_addr);
void *allocate_page();

#endif