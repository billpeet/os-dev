#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

typedef struct div
{
    int quot;
    int rem;
} div_t;

typedef struct ldiv
{
    long quot;
    long rem;
} ldiv_t;

// ### Conversion functions
// Converts string to floating point
extern double atof(const char *str);

// Converts string to int
extern int atoi(const char *str);
#define atoi(str) (int)atol(str)

// Converts string to long int
extern long atol(const char *str);
#define atol(str) strtol(str, NULL, 0)

// Converts string to double
extern double strtod(const char *str, char **endptr);

// Converts string to long int
extern long strtol(const char *str, char **endptr, int base);

// Converts string to unsigned long int
extern unsigned long strtoul(const char *str, char **endptr, int base);

// ### Memory management functions
// Allocates requested memory and returns pointer to it
extern void *malloc(size_t size);

// Allocates nitems * size and returns pointer to it
extern void *calloc(size_t nitems, size_t size);
#define calloc(nitems, size) malloc(nitems *size)

// Reallocates memory location with new size
extern void *realloc(void *ptr, size_t size);

// Deallocates memory
void free(void *ptr);

// ### Process functions
// Causes a program to exit abnormally
extern void abort(void);

// Causes function to be called on exit
extern int atexit(void (*func)(void));

// Causes program to exit with status code
extern void exit(int status);

// Gets value of specified environment variable
extern char *getenv(const char *name);

// Executes system command
extern int system(const char *str);

// Performs binary search
extern void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

// Sorts an array
extern void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));

// Returns absolute value of x
extern int abs(int x);
// Returns absolute value of x
extern long labs(long x);
#define abs(x) x >= 0 ? x : -x
#define labs(x) abs(x)

// Divides numerator by denominator
extern div_t div(int numer, int denom);
extern ldiv_t ldiv(long numer, long denom);

// Returns random int between 0 and RAND_MAX
extern int rand(void);

// Sets seed number to use on random number generator
extern void srand(unsigned int seed);

// Returns length of multibyte character pointed to by str
extern int mblen(const char *str, size_t length);

// Converts string of multibyte characters to array in pwcs
extern size_t mbstowcs(wchar_t *pwcs, const char *str, size_t length);

// Converts multibyte character string to wide character
extern int mbtowc(wchar_t *pwc, const char *str, size_t length);

// Converts the codes stored in the array pwcs to multibyte characters and stores them in the string str.
extern size_t wcstombs(char *str, const wchar_t *pwcs, size_t length);

// Examines the code which corresponds to a multibyte character given by the argument wchar.
extern int wctomb(char *str, wchar_t wchar);

#endif