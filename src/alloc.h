#ifndef ALLOC_H
#define ALLOC_H

#include "types.h"

void init_heap(void);

void *malloc(u64 size);
void free(void *ptr);

#endif