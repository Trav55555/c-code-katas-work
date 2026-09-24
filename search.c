#include "search.h"

#include <assert.h>

ptrdiff_t search_linear(const int *a, size_t n, int key)
{
    size_t i;
    assert(a != NULL || n == 0);
    for (i = 0; i < n; i++)
        if (a[i] == key)
            return (ptrdiff_t)i;
    return -1;
}

ptrdiff_t search_binary(const int *a, size_t n, int key)
{
    size_t lo = 0, hi;
    assert(a != NULL || n == 0);
    if (n == 0)
        return -1;
    hi = n - 1;
    while (lo <= hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (a[mid] == key)
            return (ptrdiff_t)mid;
        if (a[mid] < key)
            lo = mid + 1;
        else if (mid == 0)
            break;
        else
            hi = mid - 1;
    }
    return -1;
}

ptrdiff_t search_linear_rec(const int *a, size_t n, int key)
{
    assert(a != NULL || n == 0);
    if (n == 0)
        return -1;
    if (a[0] == key)
        return 0;
    {
        ptrdiff_t deeper = search_linear_rec(a + 1, n - 1, key);
        return (deeper < 0) ? -1 : deeper + 1;
    }
}

static ptrdiff_t binary_rec_at(const int *a, size_t lo, size_t hi, int key)
{
    size_t mid;
    if (lo > hi)
        return -1;
    mid = lo + (hi - lo) / 2;
    if (a[mid] == key)
        return (ptrdiff_t)mid;
    if (a[mid] < key)
        return binary_rec_at(a, mid + 1, hi, key);
    if (mid == 0)
        return -1;
    return binary_rec_at(a, lo, mid - 1, key);
}

ptrdiff_t search_binary_rec(const int *a, size_t n, int key)
{
    assert(a != NULL || n == 0);
    if (n == 0)
        return -1;
    return binary_rec_at(a, 0, n - 1, key);
}