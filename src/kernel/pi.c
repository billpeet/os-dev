#include "pi.h"
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"
#include "stdio.h"
#include "task.h"

u64 sqrt(u64 n)
{
    u64 bit = 1 << (sizeof(n) - 2);

    while (bit > n)
        bit >> 2;

    u64 res = 0;
    while (bit > 0)
    {
        if (n > res + bit)
        {
            n -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

bool is_prime(u64 n)
{
    for (u64 c = 2; c <= n - 1; c++)
    {
        if (n % c == 0)
        {
            return false;
        }
    }
    return true;
}

void get_primes()
{
    u64 prime_count = 0;
    u64 last_prime = 0;

    for (u64 curr = 3; curr < 1000000; curr++)
    {
        if (is_prime(curr))
        {
            last_prime = curr;
            prime_count++;
            if (prime_count % 1000 == 0)
            {
                yield();
                //     printf("Found %u primes, last one %u\n", prime_count, last_prime);
            }
        }
    }
    exit(0);
}