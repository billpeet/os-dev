#include "alloc.h"

unsigned int current = 0x0;

void *malloc(unsigned int size)
{
    void *ptr = (void *)current;
    current += size;
    return ptr;
}