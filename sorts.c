#include "sorts.h"

#include <assert.h>

static void swap_int(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

void sorts_bubble(int *a, size_t n)
{
    size_t i, end;
    int swapped = 1;
    if (n < 2)
        return;
    assert(a != NULL);
    for (end = n - 1; end > 0 && swapped; end--) {
        swapped = 0;
        for (i = 0; i < end; i++)
            if (a[i] > a[i + 1]) {
                swap_int(&a[i], &a[i + 1]);
                swapped = 1;
            }
    }
}

void sorts_insertion(int *a, size_t n)
{
    size_t i, j;
    if (n < 2)
        return;
    assert(a != NULL);
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && a[j - 1] > a[j]; j--)
            swap_int(&a[j - 1], &a[j]);
}

void sorts_selection(int *a, size_t n)
{
    size_t i, j, best;
    if (n < 2)
        return;
    assert(a != NULL);
    for (i = 0; i + 1 < n; i++) {
        best = i;
        for (j = i + 1; j < n; j++)
            if (a[j] < a[best])
                best = j;
        if (best != i)
            swap_int(&a[i], &a[best]);
    }
}

void sorts_shell(int *a, size_t n)
{
    size_t gap, i, j;
    if (n < 2)
        return;
    assert(a != NULL);
    for (gap = n / 2; gap > 0; gap /= 2)
        for (i = gap; i < n; i++)
            for (j = i; j >= gap && a[j - gap] > a[j]; j -= gap)
                swap_int(&a[j - gap], &a[j]);
}

/* Lomuto partition around a[hi]; returns the pivot's final index. */
static size_t partition(int *a, size_t lo, size_t hi)
{
    size_t i = lo, j;
    int pivot = a[hi];
    for (j = lo; j < hi; j++)
        if (a[j] < pivot) {
            swap_int(&a[i], &a[j]);
            i++;
        }
    swap_int(&a[i], &a[hi]);
    return i;
}

/* Median-of-three pivot moved to hi (deterministic variant). */
static void choose_pivot(int *a, size_t lo, size_t hi)
{
    size_t mid = lo + (hi - lo) / 2;
    if (a[mid] < a[lo])
        swap_int(&a[mid], &a[lo]);
    if (a[hi] < a[lo])
        swap_int(&a[hi], &a[lo]);
    if (a[mid] < a[hi])
        swap_int(&a[mid], &a[hi]);
}

static uint64_t rng_next(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

/* Recurse the smaller side, iterate the larger: depth <= log2 n.
 * seed == NULL selects the deterministic pivot; otherwise a uniformly
 * drawn index is swapped to hi first. */
static void quick_rec(int *a, size_t lo, size_t hi, uint64_t *seed)
{
    while (lo < hi) {
        size_t p;
        if (seed == NULL)
            choose_pivot(a, lo, hi);
        else
            swap_int(&a[lo + (size_t)(rng_next(seed) % (hi - lo + 1))], &a[hi]);
        p = partition(a, lo, hi);
        if (p - lo < hi - p) {
            if (p > lo)
                quick_rec(a, lo, p - 1, seed);
            lo = p + 1;
        } else {
            if (p < hi)
                quick_rec(a, p + 1, hi, seed);
            if (p == lo)
                break;
            hi = p - 1;
        }
    }
}

void sorts_quick(int *a, size_t n)
{
    if (n < 2)
        return;
    assert(a != NULL);
    quick_rec(a, 0, n - 1, NULL);
}

void sorts_quick_randomized(int *a, size_t n, uint64_t *seed)
{
    if (n < 2)
        return;
    assert(a != NULL);
    assert(seed != NULL);
    if (*seed == 0)
        *seed = UINT64_C(0x9E3779B97F4A7C15);
    quick_rec(a, 0, n - 1, seed);
}