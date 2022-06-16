#include "tester.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
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

void stdio_test()
{
    printf("printf test: 1 + 1 = %u (0x%x)\n", 2, 2);
    char str[100];
    sprintf(str, "write to string: %i", 23);
    printf("here's what was written to string: '%s'\n", str);
    puts("puts tester passed\n");

    char str2[100];
    char str3[100];
    FILE str_file;
    str_file._flag = 1;             // string type
    str_file._base = str2;          // base ptr
    str_file._ptr = str_file._base; // set ptr at start
    str_file._cnt = 100;            // zero count
    sprintf(str2, "line1\nline2");
    fgets(str3, 10, &str_file);
    printf("'%s'\n", str3);
    assert(strcmp(str3, "line1") == 0);
    str[0] = str2[0] = str3[0] = '\0';
    sscanf("abc 123 cba", "%s%s%s", str, str2, str3);
    printf("str1: %s, str2: %s, str3: %s\n", str, str2, str3);
    assert(strcmp(str, "abc") == 0);
    assert(strcmp(str2, "123") == 0);
    assert(strcmp(str3, "cba") == 0);
    int i1, i2;
    sscanf("abc 123 -222", "%s%u%i", str2, i1, i2);
    assert(strcmp(str2, "abc") == 0);
    assert(i1 == 123);
    assert(i2 = -222);
    printf("stdio tests done and passed\n");
}

void tester(void)
{
    printf("Running tests...\n");
    assert_test();
    errno_test();
    stdlib_test();
    stdio_test();
    printf("All passed.\n");
}