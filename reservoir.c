#include "reservoir.h"

#include <assert.h>

/* xorshift64*: small, fast, deterministic, seedable. */
static uint64_t rng_next(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

/* Uniform in [0, bound) via rejection sampling (no modulo bias).
 * zone is the largest multiple of bound in the 64-bit range; draws outside
 * it are rejected. bound >= 1. Expected rejections < 1 per draw. */
static uint64_t rng_below(uint64_t *state, uint64_t bound)
{
    uint64_t zone, v;
    assert(bound > 0);
    zone = (UINT64_MAX / bound) * bound;
    do {
        v = rng_next(state);
    } while (v >= zone);
    return v % bound;
}

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
