#ifndef RESERVOIR_H
#define RESERVOIR_H

#include <stddef.h>
#include <stdint.h>

/* Uniform reservoir sample (without replacement) over src[0..n).
 *
 * Contract:
 *  - Draws k = min(out_cap, n) elements into out[0..k); returns k. Only the
 *    first k slots of out are written. Selection order is emission order
 *    (not sorted).
 *  - `out` may be NULL only when out_cap == 0; `src` may be NULL only when
 *    n == 0; `seed` is non-NULL and may hold any value (0 is mixed to a
 *    fixed nonzero constant). Same *seed => same output (reproducible);
 *    *seed is advanced past the draws made. When k == 0 no draws are made
 *    and *seed is unchanged.
 *  - Selection is uniform and unbiased: the index draw uses rejection
 *    sampling, not modulo reduction.
 *  - No allocation, O(n) time, O(1) extra space beyond the output. */
size_t reservoir_sample(int *out, size_t out_cap,
                        const int *src, size_t n, uint64_t *seed);

#endif
