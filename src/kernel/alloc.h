#ifndef ALLOC_H
#define ALLOC_H

#include "types.h"
#include <stddef.h>

void init_heap(void);

void *malloc(size_t size);
void free(void *ptr);

#endif