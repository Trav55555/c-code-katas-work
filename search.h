#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>

/* Linear and binary search over int arrays.
 * Contract: `a` may be NULL only when n == 0 (returns -1). Return value is
 * the index of a matching element, or -1 when absent. search_binary and
 * search_binary_rec require `a` sorted in ascending order (precondition);
 * with duplicates any matching index may be returned. Midpoint arithmetic
 * is overflow-safe (lo + (hi - lo) / 2).
 * Recursion depth: linear_rec is O(n), binary_rec is O(log n). */
long search_linear(const int *a, size_t n, int key);
long search_binary(const int *a, size_t n, int key);
long search_linear_rec(const int *a, size_t n, int key);
long search_binary_rec(const int *a, size_t n, int key);

#endif