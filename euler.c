#include "euler.h"
#include "euler_data.h"
#include "num.h"
#include "checked.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t pow10(unsigned e)
{
    uint64_t r = 1;
    while (e > 0) {
        r *= 10;
        e--;
    }
    return r;
}

/* ---------------- PE1, PE2: series sums --------------------------------- */

/* m * (1 + 2 + ... + k) with k = (limit - 1) / m; 1 on overflow. */
static int sum_multiples_below(uint64_t m, uint64_t limit, uint64_t *out)
{
    uint64_t k, t;
    *out = 0;
    if (limit <= m)
        return 0; /* genuine empty sum, not an overflow */
    k = (limit - 1) / m;
    if (k % 2 == 0)
        t = (k / 2) * (k + 1);
    else
        t = k * ((k + 1) / 2);
    return mul_ok(t, m, out);
}

uint64_t euler_multiples_3_5(uint64_t limit)
{
    uint64_t a, b, c;
    if (sum_multiples_below(3, limit, &a) ||
        sum_multiples_below(5, limit, &b) ||
        sum_multiples_below(15, limit, &c))
        return 0; /* documented overflow indicator */
    return a + b - c; /* inclusion-exclusion */
}

uint64_t euler_even_fibonacci_sum(uint64_t limit)
{
    uint64_t a = 1, b = 2, sum = 0, next;
    while (b < limit) {
        if (b % 2 == 0) {
            if (add_ok(sum, b, &next))
                return sum; /* stop at the overflow point */
            sum = next;
        }
        if (add_ok(a, b, &next))
            break; /* Fibonacci itself overflowed */
        a = b;
        b = next;
    }
    return sum;
}

/* ---------------- PE3, PE12: factoring ----------------------------------- */

uint64_t euler_largest_prime_factor(uint64_t n)
{
    uint64_t d = 2, last = 0;
    if (n < 2)
        return 0;
    while (d <= n / d) {
        while (n % d == 0) {
            last = d;
            n /= d;
        }
        d = (d == 2) ? 3 : d + 2;
    }
    return (n > 1) ? n : last;
}

/* Divisor count of the triangle number k(k+1)/2 via its two coprime
 * halves (d(a*b) == d(a) * d(b)), with early exit once the running count
 * exceeds `min_divisors`. 12x cheaper than factoring the product. */
static int triangle_divisors_exceed(uint64_t k, unsigned min_divisors)
{
    uint64_t parts[2];
    size_t p, count = 1;

    parts[0] = (k % 2 == 0) ? k / 2 : k;
    parts[1] = (k % 2 == 0) ? k + 1 : (k + 1) / 2;
    for (p = 0; p < 2; p++) {
        uint64_t n = parts[p], d = 2;
        while (d <= n / d) {
            size_t mult = 0;
            while (n % d == 0) {
                mult++;
                n /= d;
            }
            if (mult > 0) {
                count *= mult + 1;
                if (count > (size_t)min_divisors)
                    return 1;
            }
            d = (d == 2) ? 3 : d + 2;
        }
        if (n > 1) {
            count *= 2;
            if (count > (size_t)min_divisors)
                return 1;
        }
    }
    return 0;
}

uint64_t euler_highly_divisible_triangle(unsigned min_divisors)
{
    uint64_t k = 1;
    for (;;) {
        uint64_t lo = k, hi = k + 1, t;
        if (lo % 2 == 0)
            lo /= 2;
        else
            hi /= 2;
        if (mul_ok(lo, hi, &t))
            return 0; /* triangle number would overflow */
        if (triangle_divisors_exceed(k, min_divisors))
            return t;
        k++;
    }
}

/* ---------------- PE4: palindrome product -------------------------------- */

static int is_palindrome_u64(uint64_t v)
{
    uint64_t rev = 0, t = v;
    while (t > 0) {
        rev = rev * 10 + t % 10;
        t /= 10;
    }
    return rev == v;
}

uint64_t euler_largest_palindrome_product(unsigned digits)
{
    uint64_t lo, hi, a, best = 0;
    if (digits < 1 || digits > 5)
        return 0;
    lo = pow10(digits - 1);
    hi = pow10(digits) - 1;
    for (a = hi; a >= lo; a--) {
        uint64_t b;
        if (a * hi <= best)
            break; /* no pair with this `a` can beat the best */
        for (b = hi; b >= a; b--) {
            uint64_t p = a * b;
            if (p <= best)
                break;
            if (is_palindrome_u64(p))
                best = p;
        }
        if (a == lo)
            break;
    }
    return best;
}

/* ---------------- PE5, PE6: products and sums ---------------------------- */

uint64_t euler_smallest_multiple(unsigned lo, unsigned hi)
{
    uint64_t lcm = 1, i;
    if (lo < 1 || lo > hi)
        return 0;
    for (i = lo; i <= hi; i++) {
        uint64_t g = num_gcd(lcm, i);
        uint64_t t, step = i / g;
        if (mul_ok(lcm, step, &t))
            return 0;
        lcm = t;
    }
    return lcm;
}

uint64_t euler_sum_square_difference(unsigned n)
{
    uint64_t s, q, sq, t;
    if (n == 0)
        return 0;
    /* s = n(n+1)/2, sq = n(n+1)(2n+1)/6, both halved/divided first */
    if (n % 2 == 0)
        t = (n / 2) * (n + 1);
    else
        t = n * ((n + 1) / 2);
    s = t;
    if (mul_ok(s, s, &q))
        return 0;
    /* sq = s * (2n + 1) / 3, dividing before multiplying */
    {
        uint64_t f = 2 * n + 1;
        if (f % 3 == 0) {
            f /= 3;
            if (mul_ok(s, f, &sq))
                return 0;
        } else {
            /* 3 divides s whenever it divides n(n+1)(2n+1)/6 overall */
            if (mul_ok(s / 3, f, &sq))
                return 0;
        }
    }
    return (q > sq) ? q - sq : 0;
}

/* ---------------- PE8, PE11: windowed products --------------------------- */

uint64_t euler_largest_product_in_series(size_t span)
{
    size_t i, len = sizeof euler_series_1000 - 1;
    uint64_t best = 0;
    if (span < 1 || span > 19)
        return 0; /* > 19 digits cannot bound the product in uint64 */
    for (i = 0; i + span <= len; i++) {
        uint64_t p = 1;
        size_t j;
        for (j = 0; j < span; j++)
            p *= (uint64_t)(euler_series_1000[i + j] - '0');
        if (p > best)
            best = p;
    }
    return best;
}

uint64_t euler_largest_product_in_grid(size_t span)
{
    const int dx[4] = { 1, 0, 1, 1 };
    const int dy[4] = { 0, 1, 1, -1 };
    uint64_t best = 0;
    size_t dir, x, y, k;
    if (span < 1 || span > 20)
        return 0;
    for (dir = 0; dir < 4; dir++)
        for (y = 0; y < 20; y++)
            for (x = 0; x < 20; x++) {
                uint64_t p = 1;
                int cx = (int)x, cy = (int)y;
                for (k = 0; k < span; k++) {
                    if (cx < 0 || cx > 19 || cy < 0 || cy > 19) {
                        p = 0;
                        break;
                    }
                    p *= euler_grid_20[cy][cx];
                    cx += dx[dir];
                    cy += dy[dir];
                }
                if (p > best)
                    best = p;
            }
    return best;
}

/* ---------------- PE9: Pythagorean triplet -------------------------------- */

uint64_t euler_pythagorean_triplet_product(unsigned sum)
{
    unsigned a, b;
    for (a = 1; a < sum / 3; a++)
        for (b = a + 1; b < (sum - a) / 2 + 1; b++) {
            unsigned c = sum - a - b;
            if (c > b && (uint64_t)a * a + (uint64_t)b * b == (uint64_t)c * c)
                return (uint64_t)a * b * c;
        }
    return 0;
}

/* ---------------- PE13: large sum ----------------------------------------- */

uint64_t euler_large_sum_prefix(size_t digits)
{
    enum { BASE_DIGITS = 10, LIMBS = 7 };
    uint64_t acc[LIMBS];
    const uint64_t base = UINT64_C(10000000000); /* 10^10 */
    char out[80];
    size_t i, j, top, pos = 0;

    if (digits < 1 || digits > 19)
        return 0;
    for (i = 0; i < LIMBS; i++)
        acc[i] = 0;
    for (i = 0; i < 100; i++)
        for (j = 0; j < 5; j++)
            acc[j] += euler_large_numbers[i][4 - j]; /* limb j = 10^(10j) */
    for (j = 0; j < LIMBS - 1; j++) {
        acc[j + 1] += acc[j] / base;
        acc[j] %= base;
    }
    for (top = LIMBS - 1; top > 0 && acc[top] == 0; top--)
        ;
    pos += (size_t)sprintf(out + pos, "%llu", (unsigned long long)acc[top]);
    for (j = top; j > 0; j--)
        pos += (size_t)sprintf(out + pos, "%0*llu", BASE_DIGITS,
                               (unsigned long long)acc[j - 1]);
    out[digits] = '\0';
    return strtoull(out, NULL, 10);
}

/* ---------------- PE14: Collatz ------------------------------------------- */

uint64_t euler_longest_collatz(uint64_t limit)
{
    uint32_t *cache;
    uint64_t best_start = 1;
    size_t best_len = 0, i;
    uint64_t path[4096]; /* backfill buffer for sub-limit chain values */

    if (limit < 2)
        return 0;
    cache = calloc((size_t)limit, sizeof *cache); /* 0 = not computed */
    for (i = 1; i < (size_t)limit; i++) {
        uint64_t v = (uint64_t)i;
        size_t len = 0, npath = 0, j;
        size_t at[4096];
        while (v != 1) {
            if (cache != NULL && v < limit && cache[v] != 0) {
                len += cache[v];
                break;
            }
            if (cache != NULL && v < limit && npath < 4096) {
                path[npath] = v;
                at[npath] = len; /* step offset: chains may spike above limit */
                npath++;
            }
            if (v % 2 == 0)
                v /= 2;
            else if (v <= (UINT64_MAX - 1) / 3)
                v = 3 * v + 1;
            else
                break; /* chain escaped uint64: unreachable for sane limits */
            len++;
        }
        if (cache != NULL) {
            for (j = 0; j < npath; j++)
                cache[path[j]] = (uint32_t)(len - at[j]);
        }
        if (len > best_len) {
            best_len = len;
            best_start = (uint64_t)i;
        }
    }
    free(cache); /* NULL-safe: uncached runs degrade to plain counting */
    return best_start;
}

/* ---------------- PE15, PE16: binomial and big powers --------------------- */

uint64_t euler_lattice_paths(unsigned w, unsigned h)
{
    uint64_t n = (uint64_t)w + h, k = (w < h) ? w : h, res = 1, i;
    for (i = 1; i <= k; i++) {
        uint64_t num = n - k + i, t;
        if (mul_ok(res, num, &t))
            return 0;
        res = t / i; /* binomial partial products stay divisible */
    }
    return res;
}

uint64_t euler_power_digit_sum(unsigned exponent)
{
    size_t ndigits, i, carry, sum = 0;
    unsigned step;
    unsigned char *digits;

    if (exponent > 200000)
        return 0;
    ndigits = ((size_t)exponent * 302) / 1000 + 2; /* log10(2) < 0.302, +2 slack */
    digits = calloc(ndigits, 1);
    if (digits == NULL)
        return 0;
    digits[0] = 1;
    for (step = 0; step < exponent; step++) {
        carry = 0;
        for (i = 0; i < ndigits; i++) {
            size_t v = (size_t)digits[i] * 2 + carry;
            digits[i] = (unsigned char)(v % 10);
            carry = v / 10;
        }
    }
    for (i = 0; i < ndigits; i++)
        sum += digits[i];
    free(digits);
    return (uint64_t)sum;
}

/* ---------------- PE17: number names -------------------------------------- */

static const char *const ones[20] = {
    "", "one", "two", "three", "four", "five", "six", "seven", "eight",
    "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen",
    "sixteen", "seventeen", "eighteen", "nineteen"
};
static const char *const tens[10] = {
    "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy",
    "eighty", "ninety"
};

/* Emit the British name of 1 <= n <= 999 (or the word "zero") and return
 * its letter count (spaces and hyphens are written but not counted). */
static size_t name_letters_999(unsigned n, char *buf, size_t cap, size_t *pos)
{
    size_t needed = 0;
#define EMIT(str) \
    do { \
        const char *w_ = (str); \
        size_t l_ = strlen(w_); \
        if (buf != NULL && *pos + l_ <= cap) { \
            memcpy(buf + *pos, w_, l_); \
            *pos += l_; \
        } \
        needed += l_; \
    } while (0)
#define EMITC(ch) \
    do { \
        if (buf != NULL && *pos + 1 <= cap) { \
            buf[*pos] = (ch); \
            *pos += 1; \
        } \
    } while (0)

    if (n >= 100) {
        EMIT(ones[n / 100]);
        EMITC(' ');
        EMIT("hundred");
        n %= 100;
        if (n != 0) {
            EMITC(' ');
            EMIT("and");
            EMITC(' ');
        }
    }
    if (n >= 20) {
        EMIT(tens[n / 10]);
        if (n % 10 != 0) {
            EMITC('-');
            EMIT(ones[n % 10]);
        }
    } else if (n > 0) {
        EMIT(ones[n]);
    }
#undef EMIT
#undef EMITC
    return needed;
}

/* Shared writer for the full range 0 <= n <= 9999. British "and" appears
 * after a hundred remainder and after "thousand" when the remainder is
 * below 100. Returns the letter count. */
static size_t write_name(unsigned n, char *buf, size_t cap, size_t *pos)
{
    size_t letters = 0;
    if (n == 0) {
        /* "zero" */
        if (buf != NULL && *pos + 4 <= cap) {
            memcpy(buf + *pos, "zero", 4);
            *pos += 4;
        }
        return 4;
    }
    if (n >= 1000) {
        letters += name_letters_999(n / 1000, buf, cap, pos);
        if (buf != NULL && *pos + 1 <= cap) {
            buf[*pos] = ' ';
            *pos += 1;
        }
        if (buf != NULL && *pos + 8 <= cap) {
            memcpy(buf + *pos, "thousand", 8);
            *pos += 8;
        }
        letters += 8;
        n %= 1000;
        if (n != 0 && n < 100) {
            if (buf != NULL && *pos + 5 <= cap) {
                memcpy(buf + *pos, " and ", 5);
                *pos += 5;
            }
            letters += 3;
        }
        if (n >= 100 && buf != NULL && *pos + 1 <= cap) {
            buf[*pos] = ' ';
            *pos += 1;
        }
    }
    if (n > 0)
        letters += name_letters_999(n, buf, cap, pos);
    return letters;
}

size_t euler_number_to_words(unsigned n, char *buf, size_t cap, size_t *needed)
{
    size_t pos = 0;
    assert(needed != NULL);
    assert(buf != NULL || cap == 0);
    if (n > 9999) {
        *needed = 0;
        return 0;
    }
    *needed = write_name(n, buf, cap, &pos);
    if (buf != NULL && pos < cap)
        buf[pos] = '\0';
    return (buf != NULL && pos < cap) ? pos : cap;
}

size_t euler_number_name_letters(unsigned n)
{
    size_t pos = 0;
    if (n > 9999)
        return 0;
    return write_name(n, NULL, 0, &pos);
}

uint64_t euler_number_letter_counts(unsigned lo, unsigned hi)
{
    uint64_t sum = 0;
    unsigned i;
    if (lo < 1 || lo > hi || hi > 1000000)
        return 0;
    for (i = lo; i <= hi; i++)
        sum += (uint64_t)euler_number_name_letters(i);
    return sum;
}

/* ---------------- PE18: maximum path sum ---------------------------------- */

uint64_t euler_maximum_path_sum(const unsigned *values,
                                const unsigned *row_len, size_t nrows)
{
    uint64_t *work;
    size_t total = 0, i, row;

    assert(values != NULL);
    assert(row_len != NULL);
    for (i = 0; i < nrows; i++) {
        assert(row_len[i] == i + 1); /* triangle shape */
        total += row_len[i];
    }
    if (nrows == 0)
        return 0;
    work = malloc(total * sizeof *work);
    if (work == NULL)
        return 0;
    for (i = 0; i < total; i++)
        work[i] = values[i];
    /* bottom-up: fold each row into the row above */
    for (row = nrows - 1; row > 0; row--) {
        size_t base = row * (row + 1) / 2;
        size_t above = (row - 1) * row / 2;
        size_t k;
        for (k = 0; k < row_len[row - 1]; k++) {
            uint64_t l = work[base + k], r = work[base + k + 1];
            work[above + k] += (l > r) ? l : r;
        }
    }
    {
        uint64_t result = work[0];
        free(work);
        return result;
    }
}

/* ---------------- sieve ---------------------------------------------------- */

int euler_sieve(unsigned limit, unsigned *primes, size_t cap, size_t *count)
{
    unsigned char *composite;
    size_t needed = 0, i;
    unsigned v;

    assert(count != NULL);
    assert(primes != NULL || cap == 0);
    if (limit < 2) {
        *count = 0;
        return 0;
    }
    composite = calloc((size_t)limit + 1, 1);
    if (composite == NULL)
        return -1;
    for (v = 2; (uint64_t)v * v <= limit; v++)
        if (!composite[v]) {
            uint64_t m;
            for (m = (uint64_t)v * v; m <= limit; m += v)
                composite[m] = 1;
        }
    for (i = 2; i <= (size_t)limit; i++)
        if (!composite[i]) {
            if (needed < cap)
                primes[needed] = (unsigned)i;
            needed++;
        }
    free(composite);
    *count = needed;
    return (needed <= cap) ? 0 : 1;
}