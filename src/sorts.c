#include "sorts.h"

#include <assert.h>

#include "rng.h"

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

/* Three-way (Dijkstra) partition around the value at a[hi]. On return
 * a[lo, *lt) < pivot, a[*lt, *gt) == pivot, and a[*gt, hi] > pivot; the
 * equal band is never empty. Grouping equal keys in one pass keeps
 * duplicate-heavy input from degrading to quadratic time. */
static void partition3(int *a, size_t lo, size_t hi, size_t *lt, size_t *gt)
{
    int pivot = a[hi];
    size_t l = lo, i = lo, g = hi + 1; /* hi + 1 <= n: no wrap */
    while (i < g) {
        if (a[i] < pivot) {
            swap_int(&a[l], &a[i]);
            l++;
            i++;
        } else if (a[i] > pivot) {
            g--;
            swap_int(&a[i], &a[g]);
        } else {
            i++;
        }
    }
    *lt = l;
    *gt = g;
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

/* Recurse the smaller side, iterate the larger: depth <= log2 n.
 * seed == NULL selects the median-of-three pivot; otherwise a uniformly
 * drawn index (rejection sampled, no modulo bias) is swapped to hi. */
static void quick_rec(int *a, size_t lo, size_t hi, uint64_t *seed)
{
    while (lo < hi) {
        size_t lt, gt;
        if (seed == NULL)
            choose_pivot(a, lo, hi);
        else
            swap_int(&a[lo + (size_t)rng_below(seed, (uint64_t)(hi - lo) + 1)],
                     &a[hi]);
        partition3(a, lo, hi, &lt, &gt);
        /* left part [lo, lt), right part [gt, hi]; parts of size < 2 are done */
        if (lt - lo < hi + 1 - gt) {
            if (lt - lo > 1)
                quick_rec(a, lo, lt - 1, seed);
            lo = gt; /* may pass hi, which ends the loop */
        } else {
            if (hi + 1 - gt > 1)
                quick_rec(a, gt, hi, seed);
            if (lt - lo < 2)
                break;
            hi = lt - 1;
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