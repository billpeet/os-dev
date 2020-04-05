#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdlib.h>
//

void assert_test()
{
    assert(1 == 1);
    // assert(1 == 2); // will throw error
}

void errno_test()
{
    errno = EDOM;
    assert(errno == EDOM);
}

void stdlib_test()
{
    assert(atoi("554433") == 554433);              // Decimal, no base specified
    assert(atol("23854") == 23854);                // Decimal, no base specified
    assert(atol("0x1182") == 0x1182);              // Hex, no base specified
    assert(atol("05544") == 05544);                // Octal, no base specified
    assert(strtoul("23543", NULL, 16) == 0x23543); // Hex, base specified
    assert(strtoul("5574", NULL, 8) == 05574);     // Octal, base specified
    assert(strtol("-54832", NULL, 0) == -54832);   // Negative decimal, no base specified
}

void tester(void)
{
    printf("Running tests...\n");
    assert_test();
    errno_test();
    stdlib_test();
    printf("All passed.\n");
}