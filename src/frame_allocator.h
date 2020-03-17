#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include "types.h"

#define PAGE_SIZE 4096

void init_paging();
u64 allocate_frame();
void init_frame_allocator();

#endif