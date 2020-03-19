#include "string.h"
#include "types.h"

int islower(char c)
{
    return c >= 97 && c <= 122;
}

int isupper(char c)
{
    return c >= 65 && c <= 90;
}

char toupper(char c)
{
    if (islower(c))
        c += 'A' - 'a';
    return c;
}

char tolower(char c)
{
    if (isupper(c))
        c -= 'A' - 'a';
    return c;
}

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

int strcmp(const char *str1, const char *str2)
{
    u8 c1, c2;
    while (1)
    {
        c1 = *str1++;
        c2 = *str2++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;
    do
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
    } while (c1 == c2 && c1 != 0);
    return c1 - c2;
}

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;
    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
}