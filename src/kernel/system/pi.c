#include "pi.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../task.h"

uint64_t sqrt(uint64_t n)
{
    uint64_t bit = 1 << (sizeof(n) - 2);

    while (bit > n)
        bit >> 2;

    uint64_t res = 0;
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

bool is_prime(uint64_t n)
{
    for (uint64_t c = 2; c <= n - 1; c++)
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
    uint64_t prime_count = 0;
    uint64_t last_prime = 0;

    for (uint64_t curr = 3; curr < 1000000; curr++)
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