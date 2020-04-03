#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(cond) \
    if (!(cond))     \
    panic("Assertion '" #cond "' failed.\n")
#endif

#endif