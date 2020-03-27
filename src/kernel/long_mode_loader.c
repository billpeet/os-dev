#include "types.h"
#include <string.h>

void error(unsigned char error_code)
{
    char *vidptr = (char *)0xb8000;
    char *str = "ERR: ";
    int i = 0;
    for (; str[i] != '\0'; i++)
    {
        vidptr[i * 2] = str[i];
        vidptr[i * 2 + 1] = 0x4f;
    }
    vidptr[i * 2] = error_code + 48;
    vidptr[i * 2 + 1] = 0x4f;

    asm volatile("hlt");
}