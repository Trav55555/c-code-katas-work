/* Boundary, differential, and known-answer tests for the second kata set
 * (sorts, search, dlist, bst, num, strs, recmath, euler). */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

#include "sorts.h"
#include "search.h"
#include "dlist.h"
#include "bst.h"
#include "num.h"
#include "strs.h"
#include "recmath.h"
#include "euler.h"
#include "euler_data.h"

static int failures = 0;
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); \
            failures++; \
        } \
    } while (0)

static uint64_t t_state = UINT64_C(0xFEEDFACE0DDBA11);
static uint64_t t_next(void)
{
    t_state ^= t_state >> 12;
    t_state ^= t_state << 25;
    t_state ^= t_state >> 27;
    return t_state * UINT64_C(2685821657736338717);
}

static void ref_sort(int *a, size_t n)
{
    size_t i, j;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && a[j - 1] > a[j]; j--) {
            int t = a[j - 1];
            a[j - 1] = a[j];
            a[j] = t;
        }
}

/* ---------------- sorts --------------------------------------------------- */

typedef void (*SortFn)(int *, size_t);

static void check_all_sorts(const int *src, size_t n)
{
    static const SortFn fns[5] = {
        sorts_bubble, sorts_insertion, sorts_selection, sorts_shell, sorts_quick
    };
    int want[300], got[300];
    size_t f, i;
    CHECK(n <= 300);
    memcpy(want, src, n * sizeof want[0]);
    ref_sort(want, n);
    for (f = 0; f < 5; f++) {
        memcpy(got, src, n * sizeof got[0]);
        fns[f](got, n);
        CHECK(memcmp(got, want, n * sizeof got[0]) == 0);
    }
    for (i = 0; i < 3; i++) { /* randomized variant over three seeds */
        uint64_t seed = 7 + i;
        memcpy(got, src, n * sizeof got[0]);
        sorts_quick_randomized(got, n, &seed);
        CHECK(memcmp(got, want, n * sizeof got[0]) == 0);
    }
}

static void test_sorts(void)
{
    int one[1] = { 5 }, two[2] = { 2, 1 }, sorted[6] = { 1, 2, 3, 4, 5, 6 };
    int rev[6] = { 6, 5, 4, 3, 2, 1 }, same[6] = { 7, 7, 7, 7, 7, 7 };
    int dups[7] = { 3, 1, 3, 2, 1, 2, 3 }, rnd[257];
    size_t i, trial;

    /* documented no-ops */
    sorts_bubble(NULL, 0);
    sorts_insertion(NULL, 0);
    sorts_selection(NULL, 0);
    sorts_shell(NULL, 0);
    sorts_quick(NULL, 0);
    { uint64_t s = 1; sorts_quick_randomized(NULL, 0, &s); }
    CHECK(one[0] == 5);
    check_all_sorts(one, 1);
    check_all_sorts(two, 2);
    check_all_sorts(sorted, 6);
    check_all_sorts(rev, 6);
    check_all_sorts(same, 6);
    check_all_sorts(dups, 7);

    for (trial = 0; trial < 40; trial++) {
        size_t n = (size_t)(t_next() % 258);
        for (i = 0; i < n; i++)
            rnd[i] = (int)(t_next() % 50) - 25;
        check_all_sorts(rnd, n);
    }

    /* randomized variant: same seed reproduces the same permutation work */
    {
        int a[9] = { 5, 3, 9, 1, 4, 8, 2, 7, 6 }, b[9];
        uint64_t s1 = 99, s2 = 99;
        memcpy(b, a, sizeof a);
        sorts_quick_randomized(a, 9, &s1);
        sorts_quick_randomized(b, 9, &s2);
        CHECK(s1 == s2 && memcmp(a, b, sizeof a) == 0);
    }

    /* Duplicate-heavy input must not go quadratic. A two-way partition
     * takes ~50 s on 300k equal keys; three-way takes milliseconds. The
     * budget is generous enough for sanitizer builds. */
    {
        enum { BIG = 300000 };
        static int big[BIG];
        size_t pass;
        for (pass = 0; pass < 4; pass++) {
            uint64_t seed = 7;
            clock_t start;
            double secs;
            int ok = 1;
            for (i = 0; i < BIG; i++)
                big[i] = (pass % 2 == 0) ? 7 : (int)(t_next() % 3);
            start = clock();
            if (pass < 2)
                sorts_quick(big, BIG);
            else
                sorts_quick_randomized(big, BIG, &seed);
            secs = (double)(clock() - start) / CLOCKS_PER_SEC;
            for (i = 1; i < BIG; i++)
                ok &= big[i - 1] <= big[i];
            CHECK(ok);
            CHECK(secs < 2.0);
        }
    }
}

/* ---------------- search -------------------------------------------------- */

static void test_search(void)
{
    int none[1] = { 4 };
    int unsorted[3] = { 3, 1, 2 };
    int sorted[8] = { 1, 3, 3, 5, 7, 9, 11, 13 };
    int big[1000];
    size_t i;

    CHECK(search_linear(NULL, 0, 1) == -1);
    CHECK(search_binary(NULL, 0, 1) == -1);
    CHECK(search_linear_rec(NULL, 0, 1) == -1);
    CHECK(search_binary_rec(NULL, 0, 1) == -1);

    CHECK(search_linear(none, 1, 4) == 0 && search_linear(none, 1, 5) == -1);
    CHECK(search_binary(none, 1, 4) == 0 && search_binary(none, 1, 5) == -1);
    CHECK(search_linear(unsorted, 3, 1) == 1);   /* linear needs no order */
    CHECK(search_linear_rec(unsorted, 3, 1) == 1);

    /* binary boundaries: ends, missing-below, missing-above, duplicates */
    CHECK(search_binary(sorted, 8, 1) == 0);
    CHECK(search_binary(sorted, 8, 13) == 7);
    CHECK(search_binary(sorted, 8, 0) == -1);
    CHECK(search_binary(sorted, 8, 14) == -1);
    CHECK(search_binary(sorted, 8, 6) == -1);
    {
        long idx = search_binary(sorted, 8, 3);
        CHECK(idx == 1 || idx == 2); /* either duplicate */
    }
    CHECK(search_binary_rec(sorted, 8, 1) == 0);
    CHECK(search_binary_rec(sorted, 8, 13) == 7);
    CHECK(search_binary_rec(sorted, 8, 4) == -1);
    {
        long idx = search_binary_rec(sorted, 8, 3);
        CHECK(idx == 1 || idx == 2);
    }

    /* differential: rec and iterative agree on deterministic sorted data */
    for (i = 0; i < 1000; i++)
        big[i] = (int)i * 2;
    for (i = 0; i < 200; i++) {
        int key = (int)(t_next() % 2100);
        long l1 = search_binary(big, 1000, key);
        long l2 = search_binary_rec(big, 1000, key);
        long l3 = search_linear(big, 1000, key);
        long l4 = search_linear_rec(big, 1000, key);
        CHECK((l1 < 0) == (l3 < 0));  /* found/not-found agreement */
        CHECK((l1 < 0) == (l2 < 0));
        CHECK(l3 == l4);
        if (l1 >= 0)
            CHECK(big[l1] == key && l2 >= 0 && big[l2] == key);
    }
}

/* ---------------- dlist --------------------------------------------------- */

static void test_dlist(void)
{
    DList *l;
    size_t i;
    const int *p;

    CHECK(dlist_length((l = dlist_create())) == 0);
    CHECK(l != NULL);
    if (l == NULL)
        return;
    CHECK(dlist_get(l, 0) == NULL && dlist_get_from_end(l, 0) == NULL);
    CHECK(dlist_remove_first(l, 1) == 0);
    for (i = 0; i < 5; i++)
        CHECK(dlist_append(l, (int)i) == 0);
    CHECK(dlist_length(l) == 5);
    CHECK(*dlist_get(l, 0) == 0 && *dlist_get(l, 4) == 4);
    CHECK(*dlist_get_from_end(l, 0) == 4);          /* tail */
    CHECK(*dlist_get_from_end(l, 4) == 0);          /* head */
    CHECK(*dlist_get_from_end(l, 2) == 2);          /* middle */
    CHECK(dlist_get_from_end(l, 5) == NULL);        /* out of range */

    /* forward/backward consistency */
    for (i = 0; i < 5; i++)
        CHECK(*dlist_get(l, i) == *dlist_get_from_end(l, 4 - i));

    CHECK(dlist_remove_first(l, 0) == 1);  /* head */
    CHECK(dlist_remove_first(l, 4) == 1);  /* tail */
    CHECK(dlist_remove_first(l, 2) == 1);  /* middle */
    CHECK(dlist_length(l) == 2);
    CHECK(*dlist_get(l, 0) == 1 && *dlist_get(l, 1) == 3);
    CHECK(*dlist_get_from_end(l, 0) == 3 && *dlist_get_from_end(l, 1) == 1);
    CHECK(dlist_remove_first(l, 2) == 0);  /* missing */
    p = dlist_get(l, 0);
    CHECK(p != NULL && *p == 1);
    dlist_destroy(l);
    dlist_destroy(NULL);
}

/* ---------------- bst ----------------------------------------------------- */

static int walk_seen[64];
static size_t walk_count;
static void collect_visit(int value, void *user)
{
    (void)user;
    if (walk_count < 64)
        walk_seen[walk_count] = value;
    walk_count++;
}

/* Visitor that reads the tree it is walking. With a threaded (Morris) walk
 * the lookup for 15 follows a temporary thread back up the tree forever. */
static size_t reader_visits, reader_hits;
static void reader_visit(int value, void *user)
{
    const BST *t = user;
    reader_visits++;
    if (bst_contains(t, value) && !bst_contains(t, 15) && bst_size(t) == 5)
        reader_hits++;
}

static void test_bst(void)
{
    static const int keys[7] = { 5, 3, 8, 1, 4, 7, 9 };
    BST *t;
    size_t i;

    t = bst_create();
    CHECK(t != NULL && bst_size(t) == 0);
    if (t == NULL)
        return;
    CHECK(bst_contains(t, 5) == 0);
    CHECK(bst_remove(t, 5) == 0);
    for (i = 0; i < 7; i++)
        CHECK(bst_insert(t, keys[i]) == 0);
    CHECK(bst_size(t) == 7);
    CHECK(bst_insert(t, 5) == 1);          /* duplicate: no-op */
    CHECK(bst_size(t) == 7);
    for (i = 0; i < 7; i++)
        CHECK(bst_contains(t, keys[i]) == 1);
    CHECK(bst_contains(t, 6) == 0);

    /* walk: ascending order, tree restored afterwards */
    walk_count = 0;
    bst_walk(t, collect_visit, NULL);
    CHECK(walk_count == 7);
    CHECK(walk_seen[0] == 1 && walk_seen[6] == 9);
    for (i = 1; i < 7; i++)
        CHECK(walk_seen[i - 1] < walk_seen[i]);
    CHECK(bst_size(t) == 7 && bst_contains(t, 5) == 1);

    /* removal: leaf, leaf, two-child successor, root with two children */
    CHECK(bst_remove(t, 1) == 1);   /* leaf */
    CHECK(bst_remove(t, 9) == 1);   /* leaf */
    CHECK(bst_remove(t, 3) == 1);   /* two children (successor 4) */
    CHECK(bst_size(t) == 4);
    CHECK(!bst_contains(t, 3) && bst_contains(t, 4));
    CHECK(bst_remove(t, 5) == 1);   /* root, two children */
    CHECK(bst_size(t) == 3);
    CHECK(!bst_contains(t, 5));
    walk_count = 0;
    bst_walk(t, collect_visit, NULL);
    CHECK(walk_count == 3 && walk_seen[0] < walk_seen[1] && walk_seen[1] < walk_seen[2]);
    CHECK(bst_remove(t, 5) == 0);
    bst_destroy(t);

    /* visit may read the tree being walked */
    t = bst_create();
    CHECK(t != NULL);
    if (t == NULL)
        return;
    for (i = 0; i < 5; i++)
        CHECK(bst_insert(t, (int[]){ 20, 10, 30, 12, 25 }[i]) == 0);
    reader_visits = reader_hits = 0;
    bst_walk(t, reader_visit, t);
    CHECK(reader_visits == 5 && reader_hits == 5);
    bst_destroy(t);

    /* degenerate (sorted insert) spine: exercises non-recursive teardown
     * and the walk at height n */
    t = bst_create();
    CHECK(t != NULL);
    if (t == NULL)
        return;
    for (i = 0; i < 2000; i++)
        CHECK(bst_insert(t, (int)i) == 0);
    CHECK(bst_size(t) == 2000);
    walk_count = 0;
    bst_walk(t, collect_visit, NULL);
    CHECK(walk_count == 2000 && walk_seen[0] == 0);
    CHECK(bst_contains(t, 1999) && bst_contains(t, 0) && !bst_contains(t, 2000));
    CHECK(bst_remove(t, 999) == 1 && bst_size(t) == 1999); /* spine removal */
    bst_destroy(t);
    bst_destroy(NULL);
}

/* ---------------- num ----------------------------------------------------- */

static void test_num(void)
{
    uint64_t f[128];
    size_t count;
    uint64_t a, b;

    CHECK(num_gcd(12, 18) == 6 && num_gcd(17, 5) == 1);
    CHECK(num_gcd(0, 0) == 0 && num_gcd(0, 5) == 5);
    for (a = 0; a < 60; a++)
        for (b = 0; b < 60; b++)
            CHECK(num_gcd(a, b) == num_gcd_naive(a, b));

    CHECK(num_is_prime(0) == 0 && num_is_prime(1) == 0);
    CHECK(num_is_prime(2) == 1 && num_is_prime(3) == 1 && num_is_prime(97) == 1);
    CHECK(num_is_prime(4) == 0 && num_is_prime(9) == 0 && num_is_prime(100) == 0);
    CHECK(num_is_prime(UINT64_C(4294967291)) == 1); /* prime below 2^32 */
    CHECK(num_is_prime(UINT64_C(4294967293)) == 0); /* 3 * 1431655765? composite */

    CHECK(num_factors(0, f, 128, &count) == 0 && count == 0);
    CHECK(num_factors(1, f, 128, &count) == 0 && count == 1 && f[0] == 1);
    CHECK(num_factors(12, f, 128, &count) == 0 && count == 6);
    CHECK(f[0] == 1 && f[1] == 2 && f[2] == 3 && f[3] == 4 && f[4] == 6 && f[5] == 12);
    CHECK(num_factors(36, f, 128, &count) == 0 && count == 9); /* square: pair guard */
    CHECK(f[3] == 4 && f[4] == 6 && f[5] == 9);                /* 6 = sqrt pair */
    CHECK(num_factors(12, f, 6, &count) == 0);                 /* exact fit */
    CHECK(num_factors(12, f, 5, &count) == 1 && count == 6);   /* one short */
    CHECK(num_factors(12, NULL, 0, &count) == 1 && count == 6); /* count-only */
    CHECK(num_factors(UINT64_C(1) << 30, NULL, 0, &count) == 1 && count == 31);

    CHECK(num_common_factors(12, 18, f, 128, &count) == 0 && count == 4);
    CHECK(f[0] == 1 && f[1] == 2 && f[2] == 3 && f[3] == 6);
}

/* ---------------- strs ---------------------------------------------------- */

static void test_strs(void)
{
    char buf[64];
    size_t i;

    strcpy(buf, "");
    strs_reverse(buf);
    CHECK(strcmp(buf, "") == 0);
    strcpy(buf, "a");
    strs_reverse(buf);
    CHECK(strcmp(buf, "a") == 0);
    strcpy(buf, "abc");
    strs_reverse(buf);
    CHECK(strcmp(buf, "cba") == 0);
    strs_reverse(buf);
    CHECK(strcmp(buf, "abc") == 0); /* involution */

    strcpy(buf, "banana");
    strs_replace(buf, 'a', 'o');
    CHECK(strcmp(buf, "bonono") == 0);
    strs_replace(buf, 'z', 'q');
    CHECK(strcmp(buf, "bonono") == 0); /* no-op */
    strs_replace(buf, 'o', 'o');
    CHECK(strcmp(buf, "bonono") == 0); /* identity */

    CHECK(strs_is_palindrome("") == 1);
    CHECK(strs_is_palindrome("a") == 1);
    CHECK(strs_is_palindrome("abba") == 1);
    CHECK(strs_is_palindrome("abcba") == 1);
    CHECK(strs_is_palindrome("abca") == 0);
    CHECK(strs_is_palindrome("ab") == 0);

    /* reverse involution + mirrored strings are palindromes (deterministic) */
    for (i = 0; i < 100; i++) {
        char s[24], rev[24], mirror[48];
        size_t n = (size_t)(t_next() % 23), j;
        for (j = 0; j < n; j++)
            s[j] = (char)('a' + (t_next() % 3));
        s[n] = '\0';
        strcpy(rev, s);
        strs_reverse(rev);
        strs_reverse(rev);
        CHECK(strcmp(s, rev) == 0); /* involution */
        /* mirror = s + reverse(s) reads the same both ways */
        memcpy(mirror, s, n);
        for (j = 0; j < n; j++)
            mirror[n + j] = s[n - 1 - j];
        mirror[2 * n] = '\0';
        CHECK(strs_is_palindrome(mirror) == 1);
    }
}

/* ---------------- recmath -------------------------------------------------- */

static unsigned long hanoi_moves;
static int hanoi_ok;
static unsigned hanoi_top[4][64];
static unsigned hanoi_depth[4];
static void hanoi_step(unsigned from, unsigned to, void *user)
{
    unsigned d;
    (void)user;
    hanoi_moves++;
    if (hanoi_depth[from] == 0) {
        hanoi_ok = 0;
        return;
    }
    d = hanoi_top[from][--hanoi_depth[from]];
    if (hanoi_depth[to] > 0 && hanoi_top[to][hanoi_depth[to] - 1] < d)
        hanoi_ok = 0; /* larger onto smaller */
    hanoi_top[to][hanoi_depth[to]++] = d;
}

static void test_recmath(void)
{
    uint64_t v;
    unsigned i;

    CHECK(recmath_factorial(0, &v) == 0 && v == 1);
    CHECK(recmath_factorial(5, &v) == 0 && v == 120);
    CHECK(recmath_factorial(20, &v) == 0 && v == UINT64_C(2432902008176640000));
    CHECK(recmath_factorial(21, &v) == 1); /* guard */

    CHECK(recmath_fibonacci(0, &v) == 0 && v == 0);
    CHECK(recmath_fibonacci(1, &v) == 0 && v == 1);
    CHECK(recmath_fibonacci(10, &v) == 0 && v == 55);
    CHECK(recmath_fibonacci(30, &v) == 0 && v == 832040);
    CHECK(recmath_fibonacci(93, &v) == 0 && v == UINT64_C(12200160415121876738));
    CHECK(recmath_fibonacci(94, &v) == 1); /* overflow guard */
    { /* fast doubling agrees with the naive recursion */
        uint64_t n2 = 0, n1 = 1;
        for (i = 0; i <= 24; i++) {
            uint64_t f;
            CHECK(recmath_fibonacci(i, &f) == 0 && f == n2);
            { uint64_t t = n1 + n2; n2 = n1; n1 = t; }
        }
    }

    CHECK(recmath_sum_of_digits(0) == 0 && recmath_sum_of_digits(7) == 7);
    CHECK(recmath_sum_of_digits(12345) == 15 && recmath_sum_of_digits(-12345) == 15);
    CHECK(recmath_sum_of_digits(INT64_MIN) == 89); /* |INT64_MIN| digit sum */

    CHECK(recmath_ackermann(0, 5, &v) == 0 && v == 6);
    CHECK(recmath_ackermann(2, 3, &v) == 0 && v == 9);
    CHECK(recmath_ackermann(3, 5, &v) == 0 && v == 253);
    CHECK(recmath_ackermann(3, 8, &v) == 0 && v == 2045);
    CHECK(recmath_ackermann(3, 10, &v) == 0 && v == 8189);
    CHECK(recmath_ackermann(4, 0, &v) == 0 && v == 13);
    CHECK(recmath_ackermann(3, 11, &v) == 1); /* depth ~32k frames: guard */
    CHECK(recmath_ackermann(4, 1, &v) == 1);  /* depth ~130k frames: guard */
    CHECK(recmath_ackermann(4, 2, &v) == 1);  /* value 2^65536-3: guard */
    CHECK(recmath_ackermann(3, 20, &v) == 1); /* ~50M calls: guard */

    /* Tower of Hanoi: move count, legality, and final state */
    hanoi_moves = 0;
    hanoi_ok = 1;
    hanoi_depth[1] = 3;
    hanoi_top[1][0] = 3; hanoi_top[1][1] = 2; hanoi_top[1][2] = 1;
    hanoi_depth[2] = hanoi_depth[3] = 0;
    CHECK(recmath_tower_of_hanoi(3, hanoi_step, NULL) == 0);
    CHECK(hanoi_ok == 1 && hanoi_moves == 7 && hanoi_depth[3] == 3);
    CHECK(recmath_tower_of_hanoi(0, hanoi_step, NULL) == 0);
    CHECK(recmath_tower_of_hanoi(64, hanoi_step, NULL) == 1); /* guard */
}

/* ---------------- euler ---------------------------------------------------- */

static size_t flatten_tri(const unsigned *const *rows, const unsigned *lens,
                          size_t nrows, unsigned *out)
{
    size_t i, j, k = 0;
    for (i = 0; i < nrows; i++)
        for (j = 0; j < lens[i]; j++)
            out[k++] = rows[i][j];
    return k;
}

static void test_euler(void)
{
    char words[64];
    unsigned primes[200], flat[200];
    size_t needed, count, i;
    const unsigned *tri_rows[15];
    const unsigned *ex_rows[4];

    CHECK(euler_multiples_3_5(10) == 23);
    CHECK(euler_multiples_3_5(4) == 3);      /* genuine-zero partial sums */
    CHECK(euler_multiples_3_5(1000) == 233168);            /* PE1 */
    CHECK(euler_even_fibonacci_sum(100) == 44);
    CHECK(euler_even_fibonacci_sum(4000000) == 4613732);  /* PE2 */
    CHECK(euler_largest_prime_factor(13195) == 29);
    CHECK(euler_largest_prime_factor(600851475143) == 6857); /* PE3 */
    CHECK(euler_largest_palindrome_product(2) == 9009);
    CHECK(euler_largest_palindrome_product(3) == 906609);  /* PE4 */
    CHECK(euler_smallest_multiple(1, 10) == 2520);
    CHECK(euler_smallest_multiple(1, 20) == 232792560);    /* PE5 */
    CHECK(euler_sum_square_difference(10) == 2640);
    CHECK(euler_sum_square_difference(100) == 25164150);   /* PE6 */
    CHECK(euler_largest_product_in_series(4) == 5832);    /* PE8 example */
    CHECK(euler_largest_product_in_series(13) == UINT64_C(23514624000)); /* PE8 */
    CHECK(euler_pythagorean_triplet_product(12) == 60);   /* 3-4-5 */
    CHECK(euler_pythagorean_triplet_product(1000) == UINT64_C(31875000)); /* PE9 */
    CHECK(euler_largest_product_in_grid(4) == UINT64_C(70600674)); /* PE11 */
    CHECK(euler_highly_divisible_triangle(5) == 28);
    CHECK(euler_highly_divisible_triangle(500) == UINT64_C(76576500)); /* PE12 */
    CHECK(euler_large_sum_prefix(10) == UINT64_C(5537376230)); /* PE13 */
    CHECK(euler_longest_collatz(10) == 9);
    CHECK(euler_longest_collatz(1000000) == 837799);       /* PE14 */
    CHECK(euler_lattice_paths(2, 2) == 6);
    CHECK(euler_lattice_paths(20, 20) == UINT64_C(137846528820)); /* PE15 */
    CHECK(euler_power_digit_sum(0) == 1);
    CHECK(euler_power_digit_sum(15) == 26);  /* 32768 */
    CHECK(euler_power_digit_sum(1000) == 1366);            /* PE16 */

    /* Overflow boundaries: exact at the last representable input, 0 (the
     * documented overflow indicator) just past it. References computed
     * with arbitrary-precision integers. */
    CHECK(euler_multiples_3_5(UINT64_C(8891427027)) ==
          UINT64_C(18446744066952937668)); /* a + b alone overflows here */
    CHECK(euler_multiples_3_5(UINT64_C(8891427028)) == 0);
    CHECK(euler_multiples_3_5(UINT64_C(10000000000)) == 0);
    CHECK(euler_multiples_3_5(UINT64_MAX) == 0);
    CHECK(euler_sum_square_difference(92681) == UINT64_C(18446160229542257100));
    CHECK(euler_sum_square_difference(92682) == 0);
    CHECK(euler_sum_square_difference(100000) == 0);
    CHECK(euler_sum_square_difference(UINT_MAX) == 0);
    CHECK(euler_largest_product_in_grid(10) == UINT64_C(2583621418281932160));
    CHECK(euler_largest_product_in_grid(11) == 0);
    CHECK(euler_largest_product_in_grid(20) == 0);
    CHECK(euler_lattice_paths(0, 5) == 1);
    CHECK(euler_lattice_paths(33, 33) == UINT64_C(7219428434016265740));
    CHECK(euler_lattice_paths(34, 33) == UINT64_C(14226520737620288370));
    CHECK(euler_lattice_paths(34, 34) == 0);

    CHECK(euler_number_name_letters(342) == 23);  /* PE17 example */
    CHECK(euler_number_name_letters(115) == 20);  /* PE17 example */
    CHECK(euler_number_name_letters(1000) == 11); /* "one thousand" */
    CHECK(euler_number_letter_counts(1, 1000) == 21124);  /* PE17 */
    CHECK(euler_number_letter_counts(1, 9999) > 0);
    CHECK(euler_number_letter_counts(1, 10000) == 0); /* beyond the helper range */
    CHECK(euler_number_letter_counts(9999, 9999) == euler_number_name_letters(9999));
    CHECK(euler_number_name_letters(10000) == 0);

    /* snprintf contract: *needed counts characters (spaces and hyphens too) */
    CHECK(euler_number_to_words(342, words, sizeof words, &needed) == 27);
    CHECK(needed == 27 && strcmp(words, "three hundred and forty-two") == 0);
    euler_number_to_words(115, words, sizeof words, &needed);
    CHECK(strcmp(words, "one hundred and fifteen") == 0);
    euler_number_to_words(1000, words, sizeof words, &needed);
    CHECK(strcmp(words, "one thousand") == 0);
    euler_number_to_words(1001, words, sizeof words, &needed);
    CHECK(strcmp(words, "one thousand and one") == 0);
    euler_number_to_words(1200, words, sizeof words, &needed);
    CHECK(strcmp(words, "one thousand two hundred") == 0);
    euler_number_to_words(21, words, sizeof words, &needed);
    CHECK(strcmp(words, "twenty-one") == 0);
    euler_number_to_words(0, words, sizeof words, &needed);
    CHECK(strcmp(words, "zero") == 0 && needed == 4);
    CHECK(euler_number_to_words(10000, words, sizeof words, &needed) == 0);
    CHECK(needed == 0 && words[0] == '\0');
    CHECK(euler_number_to_words(342, NULL, 0, &needed) == 0 && needed == 27);
    { /* truncation: always a NUL-terminated prefix of the full name */
        static const unsigned samples[] = { 0, 7, 21, 342, 1001, 1200, 7777, 9999 };
        char full[64], part[64];
        size_t s, cap, full_len, part_needed, wrote;
        for (s = 0; s < sizeof samples / sizeof samples[0]; s++) {
            full_len = euler_number_to_words(samples[s], full, sizeof full, &needed);
            CHECK(full_len == needed && strlen(full) == needed);
            for (cap = 1; cap <= needed + 1; cap++) {
                memset(part, '#', sizeof part);
                wrote = euler_number_to_words(samples[s], part, cap, &part_needed);
                CHECK(part_needed == needed);
                CHECK(wrote == cap - 1);
                CHECK(part[wrote] == '\0' && strncmp(part, full, wrote) == 0);
                CHECK(part[cap] == '#'); /* nothing written past cap */
            }
        }
    }

    /* PE18: the example triangle (23) and the problem triangle (1074) */
    for (i = 0; i < 4; i++)
        ex_rows[i] = euler_tri_example[i];
    i = flatten_tri(ex_rows, euler_tri_example_len, 4, flat);
    CHECK(i == 10 && euler_maximum_path_sum(flat, euler_tri_example_len, 4) == 23);
    for (i = 0; i < 15; i++)
        tri_rows[i] = euler_tri_18[i];
    i = flatten_tri(tri_rows, euler_tri_18_len, 15, flat);
    CHECK(i == 120 && euler_maximum_path_sum(flat, euler_tri_18_len, 15) == 1074); /* PE18 */

    CHECK(euler_sieve(1, primes, 200, &count) == 0 && count == 0);
    CHECK(euler_sieve(30, primes, 200, &count) == 0 && count == 10);
    CHECK(primes[0] == 2 && primes[4] == 11 && primes[9] == 29);
    CHECK(euler_sieve(1000, primes, 200, &count) == 0 && count == 168);
    CHECK(euler_sieve(30, primes, 5, &count) == 1 && count == 10); /* truncated */
}

int main(void)
{
    test_sorts();
    test_search();
    test_dlist();
    test_bst();
    test_num();
    test_strs();
    test_recmath();
    test_euler();
    printf("%s: %d failure(s)\n", failures ? "FAIL" : "OK", failures);
    return failures ? 1 : 0;
}