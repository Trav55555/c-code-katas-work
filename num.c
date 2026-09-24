#include "num.h"

#include <assert.h>

uint64_t num_gcd(uint64_t a, uint64_t b)
{
    while (b != 0) {
        uint64_t r = a % b;
        a = b;
        b = r;
    }
    return a;
}

uint64_t num_gcd_naive(uint64_t a, uint64_t b)
{
    uint64_t d = (a < b) ? a : b;
    if (d == 0)
        return (a > b) ? a : b; /* gcd(0, x) = x */
    while (d > 1) {
        if ((a % d) == 0 && (b % d) == 0)
            return d;
        d--;
    }
    return 1;
}

int num_is_prime(uint64_t n)
{
    uint64_t d;
    if (n < 2)
        return 0;
    if (n < 4)
        return 1;
    if ((n % 2) == 0)
        return 0;
    for (d = 3; d <= n / d; d += 2)
        if ((n % d) == 0)
            return 0;
    return 1;
}

/* Two-pass divisor enumeration: the small divisor d of a pair sits at
 * index i, its partner n / d at index (needed - 1 - i). No side storage. */
int num_factors(uint64_t n, uint64_t *factors, size_t cap, size_t *count)
{
    size_t needed = 0, i = 0;
    uint64_t d;

    assert(count != NULL);
    assert(factors != NULL || cap == 0);

    if (n == 0) {
        *count = 0;
        return 0;
    }
    for (d = 1; d <= n / d; d++)
        if ((n % d) == 0) {
            needed++;
            if (d != n / d)
                needed++;
        }
    *count = needed;
    if (needed > cap)
        return 1; /* count-only or too-small callers stop here */
    for (d = 1; d <= n / d; d++)
        if ((n % d) == 0) {
            factors[i] = d;
            if (d != n / d)
                factors[needed - 1 - i] = n / d;
            i++;
        }
    return 0;
}

int num_common_factors(uint64_t a, uint64_t b,
                       uint64_t *factors, size_t cap, size_t *count)
{
    /* common divisors of a and b are exactly the divisors of gcd(a, b) */
    return num_factors(num_gcd(a, b), factors, cap, count);
}