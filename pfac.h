#ifndef PFAC_H
#define PFAC_H

#include <stddef.h>
#include <stdint.h>

/* Prime factorization with multiplicity, ascending.
 *
 * Contract (snprintf-style capacity reporting):
 *  - Writes the prime factors of n (with multiplicity) in ascending order
 *    into factors[0..cap). *count is ALWAYS set to the total number of
 *    factors needed, so a caller can size the buffer with a count-only pass
 *    (factors == NULL, cap == 0).
 *  - Returns 0 when everything fit (including the n = 0 and n = 1 cases,
 *    which have zero factors), 1 when cap was too small (the first cap
 *    factors are written, *count > cap).
 *  - `factors` may be NULL only when cap == 0; `count` is non-NULL.
 *  - Trial division with the overflow-safe bound `d <= n / d` (never d*d);
 *    no allocation, no integer wrap in the loop. Cost is O(sqrt(n)) worst
 *    case: large semiprimes are slow by construction. */
int pfac_factorize(uint64_t n, uint64_t *factors, size_t cap, size_t *count);

#endif
