#ifndef SORTS_H
#define SORTS_H

#include <stddef.h>
#include <stdint.h>

/* Ascending in-place sorts of a[0..n).
 * Contract: `a` may be NULL only when n == 0; n < 2 is a no-op that
 * allocates nothing (no function here allocates). Comparison is total on
 * int. Stability: bubble and insertion are stable; selection, shell, and
 * the quick variants are not.
 * The quick variants recurse only the smaller partition and loop on the
 * larger one: stack depth is O(log n) even on adversarial input (time is
 * worst-case O(n^2) for the deterministic variant). The randomized variant
 * is deterministic for a given *seed and advances *seed. */
void sorts_bubble(int *a, size_t n);
void sorts_insertion(int *a, size_t n);
void sorts_selection(int *a, size_t n);
void sorts_shell(int *a, size_t n);
void sorts_quick(int *a, size_t n);
void sorts_quick_randomized(int *a, size_t n, uint64_t *seed);

#endif