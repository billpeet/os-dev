#ifndef ASSERT_H
#define ASSERT_H

extern void panic(const char *message, ...);

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(cond) \
    if (!(cond))     \
    panic("Assertion '" #cond "' failed.\n")
#endif

#endif