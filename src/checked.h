#ifndef CHECKED_H
#define CHECKED_H

#include <stdint.h>

/* Internal overflow-checked uint64_t arithmetic shared by the kata modules.
 * Each returns 1 (and leaves *out untouched) when the exact result does not
 * fit in uint64_t, otherwise stores it and returns 0. */
static inline int mul_ok(uint64_t a, uint64_t b, uint64_t *out)
{
    if (a != 0 && b > UINT64_MAX / a)
        return 1;
    *out = a * b;
    return 0;
}

static inline int add_ok(uint64_t a, uint64_t b, uint64_t *out)
{
    if (b > UINT64_MAX - a)
        return 1;
    *out = a + b;
    return 0;
}

#endif
