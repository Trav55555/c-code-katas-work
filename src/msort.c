#include "msort.h"

#include <assert.h>
#include <stdlib.h>

/* Merge the sorted runs [l,m) and [m,r) of `a` into `tmp`, copy back. */
static void merge_runs(int *a, int *tmp, size_t l, size_t m, size_t r)
{
    size_t i = l, j = m, k = l;
    while (i < m && j < r) {
        if (a[i] <= a[j]) /* <= keeps the sort stable */
            tmp[k++] = a[i++];
        else
            tmp[k++] = a[j++];
    }
    while (i < m)
        tmp[k++] = a[i++];
    while (j < r)
        tmp[k++] = a[j++];
    for (k = l; k < r; k++)
        a[k] = tmp[k];
}

/* One bottom-up pass over runs of length `width`. Run bounds are clamped to
 * n; i + 2*width stays representable because n <= SIZE_MAX / sizeof(int). */
static void merge_pass(int *a, int *tmp, size_t n, size_t width)
{
    size_t i;
    for (i = 0; i < n; i += 2 * width) {
        size_t m = i + width;
        size_t r = i + 2 * width;
        if (m > n)
            m = n;
        if (r > n)
            r = n;
        merge_runs(a, tmp, i, m, r);
    }
}

int msort_sort(int *a, size_t n)
{
    int *tmp;
    size_t width;

    if (n < 2)
        return 0; /* covers a == NULL with n == 0 and single-element arrays */
    assert(a != NULL);
    if (n > (size_t)-1 / sizeof *a)
        return -1; /* allocation size would overflow: array unmodified */
    tmp = malloc(n * sizeof *tmp);
    if (tmp == NULL)
        return -1; /* the only allocation; array unmodified on failure */

    for (width = 1; width < n; width *= 2) {
        merge_pass(a, tmp, n, width);
        /* width < n <= SIZE_MAX/4, so width *= 2 cannot overflow. */
    }
    free(tmp); /* single cleanup path */
    return 0;
}
