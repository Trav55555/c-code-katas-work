#ifndef RNG_H
#define RNG_H

#include <assert.h>
#include <stdint.h>

/* Internal seedable PRNG shared by the kata modules. `*state` must be
 * nonzero (xorshift has an all-zero fixed point); callers remap a zero seed
 * before the first draw. */

/* xorshift64*: small, fast, deterministic. */
static inline uint64_t rng_next(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

/* Uniform in [0, bound) via rejection sampling (no modulo bias).
 * zone is a multiple of bound; draws at or above it are rejected, so
 * expected rejections are below one per draw. bound >= 1. */
static inline uint64_t rng_below(uint64_t *state, uint64_t bound)
{
    uint64_t zone, v;
    assert(bound > 0);
    zone = (UINT64_MAX / bound) * bound;
    do {
        v = rng_next(state);
    } while (v >= zone);
    return v % bound;
}

#endif
