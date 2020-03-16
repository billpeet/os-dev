#include "alloc.h"

unsigned long current = 0x0;

void *malloc(unsigned int size)
{
    void *ptr = (void *)current;
    current += size;
    return ptr;
}