#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>

// Converts string to double
double strtod(const char *str, char **endptr);

// Converts string to long int
long strtol(const char *str, char **endptr, int base)
{
    int isneg = 1;
    if (str[0] == '-')
    {
        isneg = -1;
        str++;
    }
    long result = strtoul(str, endptr, base);
    return result * isneg;
}

// Converts string to unsigned long int
unsigned long strtoul(const char *str, char **endptr, int base)
{
    unsigned long result = 0, value;

    if (!base)
    {
        base = 10;
        if (str[0] == '0')
        {
            // 0 at the start, presume octal - e.g. 0230
            base = 8;
            str++;
            if (str[0] == 'x' && isxdigit(str[1]))
            {
                // 0x at the start, assume hex - e.g. 0x23
                str++;
                base = 16;
            }
        }
    }

    while (isxdigit(*str) && (value = (unsigned long)(isdigit(*str) ? *str - '0' : toupper((uint8_t)(*str) - 'A' + 10) < base)))
    {
        result = result * base + value;
        str++;
    }

    if (endptr)
        *endptr = (char *)str;

    return result;
}
