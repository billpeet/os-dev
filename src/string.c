#include "string.h"

unsigned int strlen(unsigned const char *str)
{
    int i = 0;
    while (str[i] != '\0')
        i++;
    return i;
}

void strrev(unsigned char *str)
{
    int i;
    int j;
    unsigned char a;
    unsigned len = strlen((const char *)str);
    for (i = 0, j = len - 1; i < j; i++, j--)
    {
        a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}