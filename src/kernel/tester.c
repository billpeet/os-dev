#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <float.h>

void assert_test()
{
    assert(1 == 1);
    //assert(1 == 2); - will throw error
}

void errno_test()
{
    errno = EDOM;
    assert(errno == EDOM);
}

void tester(void)
{
    assert_test();
    errno_test();
}