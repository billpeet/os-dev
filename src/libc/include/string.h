#ifndef STRING_H
#define STRING_H

#include "types.h"
#include <stddef.h>

// Searches for first occurrance of c in the first n bytes of str
extern void *memchr(const void *str, int c, size_t n);

// Compares the first n bytes of str1 and str2
extern int memcmp(const void *str1, const void *str2, size_t n);

// Copies n chars from src to dest
extern void *memcpy(void *dest, const void *src, size_t n);

// Moves n chars from src to dest
extern void *memmove(void *dest, const void *src, size_t n);

// Copies char c to the first n chars of str
extern void *memset(void *str, int c, size_t n);

// Appends src to dest
extern char *strcat(char *dest, const char *src);

// Appends src to dest up to n chars long
extern char *strncat(char *dest, const char *src, size_t n);

// Searches for first instance of c in str
extern char *strchr(const char *str, int c);

// Compares str1 to str2
extern int strcmp(const char *s1, const char *s2);

// Compares first n chars of str1 to str2
extern int strncmp(const char *s1, const char *s2, size_t n);

// Compares str1 to str2, result is dependent on LC_COLLATE setting of location
extern int strcoll(const char *str1, const char *str2);

// Copies src to dest
extern char *strcpy(char *dest, const char *src);

// Copies first n chars of src to dest
extern char *strncpy(char *dest, const char *src, size_t n);

// Calculates length of initial segment of str which consists of chars not in reject
extern size_t strcspn(const char *str, const char *reject);

// Searches for error number and returns pointer to error message string
extern char *strerror(int errnum);

// Returns length of string
extern unsigned int strlen(const char *str);

// Finds first char in str1 that matches str2
extern char *strpbrk(const char *str1, const char *str2);

// Searches for last occurrance of char c in str
extern char *strrchr(const char *str, int c);

// Calculates length of initial segment of str1 which consists entirely of chars from str2
extern size_t strspn(const char *str1, const char *str2);

// Returns first occurance of string needle in string haystack
extern char *strstr(const char *haystack, const char *needle);

// Breaks string str into series of tokens separated by delim
extern char *strtok(char *str, const char *delim);

// Transforms first n chars of src into current locale and places into dest
extern size_t strxfrm(char *dest, const char *src, size_t n);

extern void strrev(char *str);

extern int strcasecmp(const char *s1, const char *s2);

#endif