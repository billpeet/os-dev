#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include "types.h"

#define PAGE_SIZE 4096

#define NULLPTR (void *)0

void init_frame_allocator();

void *allocate_frame();
void deallocate_frame(void *frame);

#endif