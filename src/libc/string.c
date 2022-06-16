#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

// Searches for first occurrance of c in the first n bytes of str
void *memchr(const void *str, int c, size_t count)
{
    const unsigned char *p = str;
    while (count-- != 0)
    {
        if ((unsigned char)c == *p++)
            return (void *)(p - 1);
    }
    return NULL;
}

// Compares the first n bytes of str1 and str2
int memcmp(const void *p1, const void *p2, size_t count)
{
    const unsigned char *str1 = p1, *str2 = p2;
    int res = 0;

    for (; count > 0; str1++, str2++, count--)
    {
        if ((res = *str1 - *str2) != 0)
            break;
        return res;
    }
    return res;
}

// Copies n chars from src to dest
void *memcpy(void *dest, const void *src, size_t count)
{
    char *dstr = dest;
    const char *sstr = src;
    while (--count)
        *dstr++ = *sstr++;
    return dest;
}

// Moves n chars from src to dest - copes with overlapping areas
void *memmove(void *dest, const void *src, size_t count)
{
    char *dstr = dest;
    const char *sstr = src;
    if (dest <= src)
    {
        while (count--)
            *dstr++ = *sstr++;
    }
    else
    {
        // Copy backwards
        dstr += count;
        sstr += count;
        while (count--)
            *dstr-- = *sstr--;
    }
    return dest;
}

// Copies char c to the first n chars of str
extern void *memset(void *str, int c, size_t count)
{
    char *cstr = str;
    while (count--)
        *cstr++ = c;
    return str;
}

// Appends src to dest
extern char *strcat(char *dest, const char *src)
{
    char *rtn = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++) != '\0')
        ;
    return rtn;
}

// Appends src to dest up to n chars long
extern char *strncat(char *dest, const char *src, size_t count)
{
    char *rtn = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++) != '\0')
    {
        if (--count == 0)
        {
            *dest = '\0';
            break;
        }
    }
    return rtn;
}

// Searches for first instance of c in str
extern char *strchr(const char *str, int c)
{
    for (; *str != (char)c; ++str)
    {
        if (*str == '\0')
            return NULL;
    }
    return (char *)str;
}

// Compares str1 to str2
int strcmp(const char *str1, const char *str2)
{
    char c1, c2;
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

// Compares first n chars of str1 to str2
int strncmp(const char *str1, const char *str2, size_t n)
{
    char c1, c2;
    while (n--)
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

// Compares str1 to str2 using locale
int strcoll(const char *str1, const char *str2)
{
    // TODO
}

// Copies src to dest
char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;
    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
}

// Copies src to dest, with a maximum length
char *strncpy(char *dest, const char *src, size_t count)
{
    char *tmp = dest;
    while (count--)
    {
        if ((*tmp = *src) != 0)
            src++;
        tmp++;
        count--;
    }
    return dest;
}

// Calculates length of initial segment of string which consists of chars not in reject
size_t strcspn(const char *str, const char *reject)
{
    const char *p;
    const char *r;
    size_t count = 0;

    for (p = str; *p != '\0'; p++)
    {
        for (r = reject; *r != '\0'; r++)
        {
            if (*p == *r)
                return count;
        }
        ++count;
    }
    return count;
}

// Calculates length of initial segment of string which consists entirely of chars from accept
extern size_t strspn(const char *str, const char *accept)
{
    const char *p, *a;
    size_t count = 0;

    for (p = str; *p != '\0'; p++)
    {
        for (a = accept; *a != '\0'; a++)
        {
            if (*p == *a)
                break;
        }
        if (*a == '\0')
            return count;
        count++;
    }
    return count;
}

// Returns length of string
unsigned int strlen(const char *str)
{
    int i = 0;
    while (str[i] != '\0')
        i++;
    return i;
}

// Finds first char in str1 that matches str2
extern char *strpbrk(const char *str1, const char *str2)
{
    const char *sc1, *sc2;
    for (sc1 = str1; *sc1 != '\0'; ++sc1)
    {
        for (sc2 = str2; *sc2 != '\0'; ++sc2)
        {
            if (*sc1 == *sc2)
                return (char *)sc1;
        }
    }
}

// Searches for last occurrance of char c in str
char *strrchr(const char *str, int c)
{
    const char *last = NULL;
    do
    {
        if (*str == (char)c)
            last = str;
    } while (*str++);
    return (char *)last;
}

// Returns first occurance of string needle in string haystack
char *strstr(const char *haystack, const char *needle)
{
    size_t ln = strlen(needle);
    if (ln == 0)
        return (char *)haystack;
    size_t lh = strlen(haystack);
    while (lh >= ln)
    {
        lh--;
        if (!memcmp(haystack, needle, ln))
            return (char *)haystack;
        haystack++;
    }
    return NULL;
}

void strrev(char *str)
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
