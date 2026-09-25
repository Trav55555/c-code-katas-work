#include "reservoir.h"

#include <assert.h>

#include "rng.h"

size_t reservoir_sample(int *out, size_t out_cap,
                        const int *src, size_t n, uint64_t *seed)
{
    size_t k, j;

    assert(seed != NULL);
    assert(out != NULL || out_cap == 0);
    assert(src != NULL || n == 0);

    k = (out_cap < n) ? out_cap : n;
    if (k == 0)
        return 0; /* no draws: *seed unchanged, per contract */

    if (*seed == 0)
        *seed = UINT64_C(0x9E3779B97F4A7C15); /* xorshift state must be nonzero */

    for (j = 0; j < k; j++)
        out[j] = src[j];

    for (j = k; j < n; j++) {
        uint64_t t = rng_below(seed, (uint64_t)j + 1);
        if (t < (uint64_t)k)
            out[t] = src[j];
    }
    return k;
}
