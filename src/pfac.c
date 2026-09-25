#include "pfac.h"

#include <assert.h>

int pfac_factorize(uint64_t n, uint64_t *factors, size_t cap, size_t *count)
{
    size_t needed = 0;
    uint64_t d = 2;

    assert(count != NULL);
    assert(factors != NULL || cap == 0);

    /* Overflow-safe bound: d*d <= n is written as d <= n / d so the loop
     * bound never wraps at the top of the 64-bit range. */
    while (d <= n / d) {
        while (n % d == 0) {
            if (needed < cap)
                factors[needed] = d;
            needed++;
            n /= d;
        }
        d = (d == 2) ? 3 : d + 2; /* skip even candidates after 2 */
    }
    if (n > 1) { /* the remaining cofactor is prime */
        if (needed < cap)
            factors[needed] = n;
        needed++;
    }

    *count = needed; /* always reported, even when truncated */
    return (needed <= cap) ? 0 : 1;
}
