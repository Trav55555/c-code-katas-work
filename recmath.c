#include "recmath.h"

#include <assert.h>
#include <stddef.h>

static int mul_ok(uint64_t a, uint64_t b, uint64_t *out)
{
    if (a != 0 && b > UINT64_MAX / a)
        return 1;
    *out = a * b;
    return 0;
}

static int add_ok(uint64_t a, uint64_t b, uint64_t *out)
{
    if (b > UINT64_MAX - a)
        return 1;
    *out = a + b;
    return 0;
}

static int fact_rec(unsigned n, uint64_t *out)
{
    uint64_t sub;
    if (n <= 1) {
        *out = 1;
        return 0;
    }
    if (fact_rec(n - 1, &sub))
        return 1;
    return mul_ok(sub, (uint64_t)n, out);
}

int recmath_factorial(unsigned n, uint64_t *out)
{
    assert(out != NULL);
    if (n > 20)
        return 1; /* 21! exceeds uint64 */
    return fact_rec(n, out);
}

/* Fast doubling: returns F(n) and F(n+1) with overflow checks. */
static int fib_pair(unsigned n, uint64_t *fa, uint64_t *fb)
{
    uint64_t c, d, e, f, t, u;
    if (n == 0) {
        *fa = 0;
        *fb = 1;
        return 0;
    }
    if (fib_pair(n >> 1, &c, &d))
        return 1; /* c = F(k), d = F(k+1) with k = n >> 1 */
    if (mul_ok(d, 2, &t))       /* t = 2 * F(k+1) */
        return 1;
    t -= c;                     /* t = 2 * F(k+1) - F(k) */
    if (mul_ok(c, t, &e))       /* e = F(2k) */
        return 1;
    if (mul_ok(c, c, &f) || mul_ok(d, d, &u) || add_ok(f, u, &f))
        return 1;               /* f = F(2k+1) */
    if ((n & 1) != 0) {
        *fa = f; /* F(2k+1): the requested value, overflow-checked above */
        if (add_ok(e, f, &u))
            u = UINT64_MAX; /* F(2k+2) saturates: unused when n == 93 */
        *fb = u;
    } else {
        *fa = e;
        *fb = f;
    }
    return 0;
}

int recmath_fibonacci(unsigned n, uint64_t *out)
{
    uint64_t next;
    assert(out != NULL);
    if (n > 93)
        return 1; /* F(94) overflows uint64 */
    return fib_pair(n, out, &next);
}

int recmath_sum_of_digits(int64_t n)
{
    /* negate via uint64 so INT64_MIN is handled without UB */
    uint64_t m = (n < 0) ? (uint64_t)(-(n + 1)) + 1u : (uint64_t)n;
    if (m < 10)
        return (int)m;
    return (int)(m % 10) + recmath_sum_of_digits((int64_t)(m / 10));
}

static uint64_t ack_rec(unsigned m, uint64_t n)
{
    if (m == 0)
        return n + 1;
    if (n == 0)
        return ack_rec(m - 1, 1);
    return ack_rec(m - 1, ack_rec(m, n - 1));
}

int recmath_ackermann(unsigned m, unsigned n, uint64_t *out)
{
    assert(out != NULL);
    if (!((m <= 3 && n <= 10) || (m == 4 && n == 0)))
        return 1; /* depth and call count scale with the result */
    *out = ack_rec(m, (uint64_t)n);
    return 0;
}

static void hanoi_rec(unsigned disks, unsigned from, unsigned to,
                      unsigned via, HanoiMoveFunc move, void *user)
{
    if (disks == 0)
        return;
    hanoi_rec(disks - 1, from, via, to, move, user);
    move(from, to, user);
    hanoi_rec(disks - 1, via, to, from, move, user);
}

int recmath_tower_of_hanoi(unsigned disks, HanoiMoveFunc move, void *user)
{
    assert(move != NULL);
    if (disks > 63)
        return 1; /* 2^disks - 1 moves would exceed any budget */
    hanoi_rec(disks, 1, 3, 2, move, user);
    return 0;
}