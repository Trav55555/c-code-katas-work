/* Boundary and malformed-case tests for the five kata modules.
 * One CHECK per contract claim; failure prints file:line and the expression. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "luhn.h"
#include "slist.h"
#include "msort.h"
#include "reservoir.h"
#include "pfac.h"

static int failures = 0;
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); \
            failures++; \
        } \
    } while (0)

/* Deterministic PRNG for test data (independent of reservoir's RNG). */
static uint64_t t_state = UINT64_C(0xDEADBEEFCAFEBABE);
static uint64_t t_next(void)
{
    t_state ^= t_state >> 12;
    t_state ^= t_state << 25;
    t_state ^= t_state >> 27;
    return t_state * UINT64_C(2685821657736338717);
}

/* ---------------- luhn: untrusted input, termination, parity ------------- */

static void test_luhn(void)
{
    char *big;
    size_t i;

    CHECK(luhn_is_valid("79927398713") == 1);      /* canonical valid */
    CHECK(luhn_is_valid("79927398710") == 0);      /* single-digit change */
    CHECK(luhn_is_valid("4111111111111111") == 1); /* VISA test number */
    CHECK(luhn_is_valid("4242 4242 4242 4242") == 1); /* spaces skipped */
    CHECK(luhn_is_valid(" 4242 4242 4242 4242 ") == 1); /* leading/trailing */
    CHECK(luhn_is_valid("0") == 1);                /* one digit: sum 0 */
    CHECK(luhn_is_valid("059") == 1);              /* odd length parity */
    CHECK(luhn_is_valid("") == 0);                 /* empty: invalid */
    CHECK(luhn_is_valid("   ") == 0);              /* all spaces: invalid */
    CHECK(luhn_is_valid("12x4") == 0);             /* foreign byte */
    CHECK(luhn_is_valid("x79927398713") == 0);     /* foreign first byte */
    CHECK(luhn_is_valid("a") == 0);                /* foreign sole byte */
    CHECK(luhn_is_valid("12\t4") == 0);            /* control byte */
    CHECK(luhn_is_valid("12-4") == 0);             /* punctuation */
    CHECK(luhn_is_valid(NULL) == 0);               /* defensive total call */
    CHECK(luhn_is_valid("123") == luhn_is_valid("123")); /* deterministic */

    /* embedded NUL ends the string: "123" part decides, junk after is unseen */
    CHECK(luhn_is_valid("123\099999") == luhn_is_valid("123"));

    /* unbounded length: 100k zeros must not overflow the accumulator */
    big = malloc(100001);
    CHECK(big != NULL);
    if (big != NULL) {
        for (i = 0; i < 100000; i++)
            big[i] = '0';
        big[100000] = '\0';
        CHECK(luhn_is_valid(big) == 1);
        free(big);
    }
}

/* ---------------- slist: ownership, cleanup, failure preservation -------- */

static void test_slist(void)
{
    SList *l;
    int arr[3] = { 10, 20, 30 };
    size_t i;

    CHECK(slist_length((l = slist_create())) == 0); /* create + empty */
    CHECK(l != NULL);
    if (l == NULL)
        return;
    CHECK(slist_get(l, 0) == NULL);          /* empty: out of range */
    CHECK(slist_remove_first(l, 7) == 0);    /* remove missing */
    slist_destroy(l);

    l = slist_create();
    CHECK(l != NULL);
    if (l == NULL)
        return;
    for (i = 0; i < 3; i++)
        CHECK(slist_append(l, arr[i]) == 0); /* ordering */
    CHECK(slist_length(l) == 3);
    CHECK(slist_get(l, 0) != NULL && *slist_get(l, 0) == 10);
    CHECK(slist_get(l, 2) != NULL && *slist_get(l, 2) == 30);
    CHECK(slist_get(l, 3) == NULL);          /* boundary: index == length */

    CHECK(slist_remove_first(l, 10) == 1);   /* head */
    CHECK(slist_length(l) == 2 && *slist_get(l, 0) == 20);
    CHECK(slist_remove_first(l, 30) == 1);   /* tail */
    CHECK(slist_length(l) == 1 && *slist_get(l, 0) == 20);
    CHECK(slist_remove_first(l, 20) == 1);   /* last element */
    CHECK(slist_length(l) == 0 && slist_get(l, 0) == NULL);
    CHECK(slist_remove_first(l, 20) == 0);   /* now missing */

    /* re-fill and remove from the middle */
    for (i = 0; i < 5; i++)
        CHECK(slist_append(l, (int)i) == 0);
    CHECK(slist_remove_first(l, 2) == 1);
    CHECK(slist_length(l) == 4);
    CHECK(*slist_get(l, 0) == 0 && *slist_get(l, 1) == 1 &&
          *slist_get(l, 2) == 3 && *slist_get(l, 3) == 4);
    slist_destroy(l);

    slist_destroy(NULL); /* NULL-safe teardown */
}

/* ---------------- msort: sizing, stability, differential oracle ---------- */

static void ref_sort(int *a, size_t n) /* insertion sort: stable reference */
{
    size_t i, j;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && a[j - 1] > a[j]; j--) {
            int t = a[j - 1];
            a[j - 1] = a[j];
            a[j] = t;
        }
}

static void test_msort(void)
{
    int one[1] = { 5 };
    int two[2] = { 2, 1 };
    int dup[6] = { 3, 1, 3, 2, 1, 2 };
    int rev[5] = { 5, 4, 3, 2, 1 };
    enum { N = 257 };
    int a[N], b[N];
    size_t i, trial, n;

    CHECK(msort_sort(NULL, 0) == 0);      /* documented NULL case */
    CHECK(msort_sort(one, 0) == 0);       /* n == 0 on live pointer */
    CHECK(msort_sort(one, 1) == 0);       /* single element */
    /* scratch size would overflow: -1 before touching the array */
    {
        int guard[1] = { 7 };
        CHECK(msort_sort(guard, SIZE_MAX / sizeof(int) + 1) == -1 && guard[0] == 7);
    }
    CHECK(one[0] == 5);
    CHECK(msort_sort(two, 2) == 0 && two[0] == 1 && two[1] == 2);
    CHECK(msort_sort(dup, 6) == 0);
    CHECK(dup[0] == 1 && dup[1] == 1 && dup[2] == 2 &&
          dup[3] == 2 && dup[4] == 3 && dup[5] == 3);
    CHECK(msort_sort(rev, 5) == 0 && rev[0] == 1 && rev[4] == 5);
    CHECK(msort_sort(rev, 5) == 0 && rev[0] == 1); /* idempotent */

    /* differential test against the reference on deterministic data */
    for (trial = 0; trial < 40; trial++) {
        n = (size_t)(t_next() % (N + 1));
        for (i = 0; i < n; i++)
            a[i] = b[i] = (int)(t_next() % 20) - 10; /* duplicates likely */
        CHECK(msort_sort(a, n) == 0);
        ref_sort(b, n);
        CHECK(memcmp(a, b, n * sizeof a[0]) == 0);
    }
}

/* ---------------- reservoir: bounds, determinism, coverage --------------- */

static void test_reservoir(void)
{
    enum { N = 8, K = 3, SEEDS = 200 };
    int src[N], out1[K], out2[K], all[N], seen[N];
    uint64_t s1, s2, seed;
    size_t k, i, j, r;

    for (i = 0; i < N; i++)
        src[i] = (int)i; /* values == indices: selection is observable */

    seed = 1;
    CHECK(reservoir_sample(NULL, 0, src, N, &seed) == 0);
    CHECK(seed == 1);                        /* k == 0 consumes no draws */
    seed = 1;
    CHECK(reservoir_sample(all, N, NULL, 0, &seed) == 0);
    seed = 1;
    CHECK(reservoir_sample(all, N, src, N, &seed) == N); /* k = min: n written */
    CHECK(memcmp(all, src, sizeof all) == 0); /* cap >= n copies through */
    seed = 1;
    CHECK(reservoir_sample(out1, K, src, 2, &seed) == 2); /* n < cap */

    /* determinism: same seed => identical output and seed advance */
    s1 = s2 = 42;
    k = reservoir_sample(out1, K, src, N, &s1);
    CHECK(k == K);
    CHECK(reservoir_sample(out2, K, src, N, &s2) == K);
    CHECK(s1 == s2);
    CHECK(memcmp(out1, out2, sizeof out1) == 0);

    /* validity and coverage across fixed seeds */
    for (i = 0; i < N; i++)
        seen[i] = 0;
    for (r = 1; r <= SEEDS; r++) {
        seed = r;
        k = reservoir_sample(out1, K, src, N, &seed);
        CHECK(k == K);
        for (j = 0; j < k; j++) {
            CHECK(out1[j] >= 0 && out1[j] < N); /* in range */
            seen[out1[j]] = 1;
        }
    }
    for (i = 0; i < N; i++)
        CHECK(seen[i] == 1); /* every element selectable (not just 0..k-1) */

    /* zero seed is accepted and mixed */
    seed = 0;
    CHECK(reservoir_sample(out1, K, src, N, &seed) == K && seed != 0);
}

/* ---------------- pfac: overflow-safe bounds, capacity contract ---------- */

static void test_pfac(void)
{
    uint64_t f[80];
    size_t count;
    const uint64_t big_factors[7] = { 3, 5, 17, 257, 641, 65537, 6700417 };
    size_t i;

    CHECK(pfac_factorize(0, f, 80, &count) == 0 && count == 0); /* n = 0 */
    CHECK(pfac_factorize(1, f, 80, &count) == 0 && count == 0); /* n = 1 */
    CHECK(pfac_factorize(2, f, 80, &count) == 0 && count == 1 && f[0] == 2);
    CHECK(pfac_factorize(12, f, 80, &count) == 0 && count == 3);
    CHECK(f[0] == 2 && f[1] == 2 && f[2] == 3);       /* ascending, multiplicity */
    CHECK(pfac_factorize(97, f, 80, &count) == 0 && count == 1 && f[0] == 97);

    /* exact-fit and boundary-plus/minus-one capacities */
    CHECK(pfac_factorize(12, f, 3, &count) == 0 && count == 3);   /* exact */
    CHECK(pfac_factorize(12, f, 2, &count) == 1 && count == 3);   /* one short */
    CHECK(pfac_factorize(12, f, 4, &count) == 0);                 /* one over */
    CHECK(pfac_factorize(12, NULL, 0, &count) == 1 && count == 3); /* count-only */
    CHECK(pfac_factorize(1, NULL, 0, &count) == 0 && count == 0);  /* count-only fit */
    { /* exact-size heap buffers: ASan sees a write at factors[cap], which
       * the oversized f[80] above would absorb */
        size_t cap;
        for (cap = 0; cap <= 4; cap++) {
            uint64_t *exact = (cap > 0) ? malloc(cap * sizeof *exact) : NULL;
            if (cap > 0 && exact == NULL)
                continue;
            CHECK(pfac_factorize(12, exact, cap, &count) == (cap < 3 ? 1 : 0));
            CHECK(count == 3);
            free(exact);
        }
    }

    /* 2^63 has 63 factors of 2: exercises capacity reporting at scale */
    CHECK(pfac_factorize(UINT64_C(1) << 63, f, 80, &count) == 0 && count == 63);
    for (i = 0; i < 63; i++)
        CHECK(f[i] == 2);
    CHECK(pfac_factorize(UINT64_C(1) << 63, f, 62, &count) == 1 && count == 63);

    /* 2^64-1 = 3*5*17*257*641*65537*6700417: factors straddle 2^32, which is
     * exactly where a d*d loop bound would wrap */
    CHECK(pfac_factorize(UINT64_MAX, f, 80, &count) == 0 && count == 7);
    for (i = 0; i < 7; i++)
        CHECK(f[i] == big_factors[i]);

    /* largest prime below 2^32: loop bound must stop at sqrt, not wrap */
    CHECK(pfac_factorize(UINT64_C(4294967291), f, 80, &count) == 0);
    CHECK(count == 1 && f[0] == UINT64_C(4294967291));
}

/* Chi-square uniformity check for the rejection sampler (k = 1 over n = 6):
 * deterministic seed sequence, so the statistic is reproducible. 95%
 * critical value for 5 degrees of freedom is 11.07. A real modulo bias or a
 * broken draw pushes chi-square far past it. */
static void test_reservoir_uniformity(void)
{
    enum { N = 6, DRAWS = 12000 };
    int src[N], out[1];
    size_t counts[N], i;
    double chi = 0.0, expected = (double)DRAWS / (double)N;

    for (i = 0; i < N; i++) {
        src[i] = (int)i;
        counts[i] = 0;
    }
    for (i = 0; i < DRAWS; i++) {
        uint64_t seed = UINT64_C(0x0123456789ABCDEF) +
                        (uint64_t)i * UINT64_C(0x9E3779B97F4A7C15);
        CHECK(reservoir_sample(out, 1, src, N, &seed) == 1);
        counts[out[0]]++;
    }
    for (i = 0; i < N; i++) {
        double d = (double)counts[i] - expected;
        chi += d * d / expected;
    }
    if (chi >= 11.07)
        printf("  chi-square = %.2f (critical 11.07)\n", chi);
    CHECK(chi < 11.07);
}

int main(void)
{
    test_luhn();
    test_slist();
    test_msort();
    test_reservoir();
    test_reservoir_uniformity();
    test_pfac();
    printf("%s: %d failure(s)\n", failures ? "FAIL" : "OK", failures);
    return failures ? 1 : 0;
}
