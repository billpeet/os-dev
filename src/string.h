#ifndef STRING_H
#define STRING_H

#include "types.h"

unsigned int strlen(unsigned const char *str);

void strrev(unsigned char *str);

int strcmp(const char *s1, const char *s2);

int strcasecmp(const char *s1, const char *s2);

char *strcpy(char *dest, const char *src);

char toupper(char c);
char tolower(char c);

#endif