#ifndef MSORT_H
#define MSORT_H

#include <stddef.h>

/* Stable ascending merge sort of a[0..n), in place.
 *
 * Contract:
 *  - `a` may be NULL only when n == 0. n may be 0 or 1 (no-op, no allocation).
 *  - Allocates one scratch buffer of n ints; on allocation failure (or if
 *    n * sizeof(int) would overflow size_t) the array is left UNMODIFIED and
 *    -1 is returned. Otherwise returns 0 and the array is sorted.
 *  - Iterative bottom-up merging: no recursion, so no stack-depth bound to
 *    reason about. O(n) scratch, O(n log n) time. Ties take the left run
 *    (the stable choice), but with bare int keys stability is not
 *    observable, so it is not part of this contract.
 *  - Element type is int; comparison is total on int (no NaN domain). */
int msort_sort(int *a, size_t n);

#endif
