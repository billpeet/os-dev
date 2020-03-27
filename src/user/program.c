#include <stdio.h>

void main()
{
    int pos = 0;
    char *vidptr = (char *)0xb8000;
    vidptr[pos * 2] = 'F';
    vidptr[pos * 2 + 1] = 0x07;
}