/* Bounded fuzzing for the modules' input surfaces, run under ASan+UBSan.
 * Oracles: luhn returns only {0,1}; reservoir outputs are distinct and in
 * range; msort matches a reference insertion sort (differential); slist
 * matches an array model; pfac matches a reference factorization and
 * reconstructs its input by multiplication. Deterministic PRNG corpus.
 * Reports iteration counts. Demonstrates explored behavior for this corpus
 * and budget only -- never exhaustive safety. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "luhn.h"
#include "slist.h"
#include "msort.h"
#include "reservoir.h"
#include "pfac.h"
#include "sorts.h"
#include "strs.h"
#include "search.h"

static uint64_t f_state = UINT64_C(0x0123456789ABCDEF);
static uint64_t f_next(void)
{
    f_state ^= f_state >> 12;
    f_state ^= f_state << 25;
    f_state ^= f_state >> 27;
    return f_state * UINT64_C(2685821657736338717);
}

static int failures = 0;
static void fail(const char *what)
{
    if (failures < 5)
        printf("FAIL %s\n", what);
    failures++;
}

/* ---------------- luhn: arbitrary bytes, several alphabets --------------- */

static void fuzz_luhn(size_t iters)
{
    enum { BUF = 512 };
    static const char *alphabets[] = {
        "0123456789  ",
        "0123456789 abcxyz-+\t",
        "\x01\x02\x7f\x80\xff 09",
    };
    char buf[BUF + 1];
    size_t it, j;

    for (it = 0; it < iters; it++) {
        const char *alpha = alphabets[f_next() % 3];
        size_t alpha_len = strlen(alpha);
        size_t len = (size_t)(f_next() % (BUF + 1));
        int r;
        for (j = 0; j < len; j++)
            buf[j] = alpha[f_next() % alpha_len];
        buf[len] = '\0';
        r = luhn_is_valid(buf);
        if (r != 0 && r != 1)
            fail("luhn return outside {0,1}");
    }
    printf("fuzz luhn: %u iters, 3 alphabets, len 0..%d\n",
           (unsigned)iters, BUF);
}

/* ---------------- reservoir: range and no-replacement invariants --------- */

static void fuzz_reservoir(size_t iters)
{
    enum { NELEM = 64 };
    int src[NELEM], out[NELEM];
    size_t it, j, k, n, cap;

    for (it = 0; it < iters; it++) {
        uint64_t seed = f_next();
        n = (size_t)(f_next() % (NELEM + 1));
        cap = (size_t)(f_next() % (NELEM + 1));
        for (j = 0; j < n; j++)
            src[j] = (int)j;
        k = reservoir_sample(out, cap, src, n, &seed);
        if (k != ((cap < n) ? cap : n))
            fail("reservoir return != min(cap, n)");
        for (j = 0; j < k; j++) {
            size_t m;
            if (out[j] < 0 || (size_t)out[j] >= n)
                fail("reservoir output out of range");
            for (m = 0; m < j; m++)
                if (out[m] == out[j])
                    fail("reservoir repeated an element");
        }
    }
    printf("fuzz reservoir: %u iters, n/cap 0..%d\n", (unsigned)iters, NELEM);
}

/* ---------------- msort: differential against reference sort ------------- */

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

static void fuzz_msort(size_t iters)
{
    enum { N = 400 };
    int a[N], b[N];
    size_t it, i, n;

    for (it = 0; it < iters; it++) {
        n = (size_t)(f_next() % (N + 1));
        for (i = 0; i < n; i++)
            a[i] = b[i] = (int)(f_next() % 11) - 5; /* heavy duplicates */
        if (msort_sort(a, n) != 0)
            fail("msort returned -1 with memory available");
        ref_sort(b, n);
        if (memcmp(a, b, n * sizeof a[0]) != 0)
            fail("msort mismatch vs reference");
    }
    printf("fuzz msort: %u iters, n 0..%d, differential vs reference\n",
           (unsigned)iters, N);
}

/* ---------------- slist: model checking against an array ---------------- */

static void fuzz_slist(size_t iters)
{
    enum { MODEL = 64, OPS = 128 };
    int model[MODEL];
    size_t it, op, nm = 0, i, idx;

    for (it = 0; it < iters; it++) {
        SList *l = slist_create();
        nm = 0;
        if (l == NULL)
            continue; /* memory pressure at random: skip this iteration */
        for (op = 0; op < OPS; op++) {
            switch (f_next() % 4) {
            case 0:
                if (nm < MODEL) {
                    int v = (int)(f_next() % 8);
                    if (slist_append(l, v) != 0)
                        fail("slist append failed with memory available");
                    model[nm++] = v;
                }
                break;
            case 1: {
                int v = (int)(f_next() % 8);
                size_t m;
                int expect = 0;
                size_t found = (size_t)-1;
                for (m = 0; m < nm; m++)
                    if (model[m] == v) {
                        found = m;
                        expect = 1;
                        break;
                    }
                if (slist_remove_first(l, v) != expect)
                    fail("slist remove_first mismatch");
                if (expect) {
                    for (i = found; i + 1 < nm; i++)
                        model[i] = model[i + 1];
                    nm--;
                }
                break;
            }
            case 2:
                idx = (size_t)(f_next() % (MODEL + 1));
                if (slist_length(l) != nm)
                    fail("slist length mismatch");
                if (idx >= nm) {
                    if (slist_get(l, idx) != NULL)
                        fail("slist get out of range");
                } else if (slist_get(l, idx) == NULL ||
                           *slist_get(l, idx) != model[idx]) {
                    fail("slist get value mismatch");
                }
                break;
            default:
                if (slist_length(l) != nm)
                    fail("slist length mismatch");
                break;
            }
        }
        slist_destroy(l);
    }
    printf("fuzz slist: %u iters, %u ops each, model-checked\n",
           (unsigned)iters, (unsigned)OPS);
}

/* ---------------- pfac: reference factorization + product oracle --------- */

static void ref_factorize(uint64_t n, uint64_t *f, size_t *count)
{
    size_t needed = 0;
    uint64_t d;
    for (d = 2; d * d <= n; d++) { /* oracle: n is bounded, d*d cannot wrap */
        while (n % d == 0) {
            f[needed++] = d;
            n /= d;
        }
    }
    if (n > 1)
        f[needed++] = n;
    *count = needed;
}

static void fuzz_pfac(size_t iters)
{
    uint64_t f[80], rf[80], product;
    size_t it, count, rcount, cap, i;

    for (it = 0; it < iters; it++) {
        /* bounded n keeps trial division cheap; the 2^64 edges live in tests */
        uint64_t n = f_next() % UINT64_C(1000000);
        int ret;
        cap = (size_t)(f_next() % 71);
        ref_factorize(n, rf, &rcount);
        ret = pfac_factorize(n, f, cap, &count);
        if (count != rcount)
            fail("pfac count mismatch vs reference");
        if (ret != ((count <= cap) ? 0 : 1))
            fail("pfac return code mismatch");
        if (count <= cap) {
            if (memcmp(f, rf, count * sizeof f[0]) != 0)
                fail("pfac factors mismatch vs reference");
            product = 1;
            for (i = 0; i < count; i++)
                product *= f[i];
            if (count > 0 && product != n)
                fail("pfac product oracle mismatch");
        } else {
            for (i = 0; i < cap; i++)
                if (f[i] != rf[i])
                    fail("pfac truncated prefix mismatch");
        }
    }
    printf("fuzz pfac: %u iters, n < 10^6, cap 0..70, reference oracle\n",
           (unsigned)iters);
}

/* ---------------- sorts: differential vs reference ----------------------- */

static void fuzz_sorts(size_t iters)
{
    enum { N = 300 };
    int a[N], b[N];
    size_t it, i, n;

    for (it = 0; it < iters; it++) {
        uint64_t seed = f_next();
        n = (size_t)(f_next() % (N + 1));
        for (i = 0; i < n; i++)
            a[i] = b[i] = (int)(f_next() % 21) - 10;
        ref_sort(b, n);
        switch (it % 6) {
        case 0: sorts_bubble(a, n); break;
        case 1: sorts_insertion(a, n); break;
        case 2: sorts_selection(a, n); break;
        case 3: sorts_shell(a, n); break;
        case 4: sorts_quick(a, n); break;
        default: sorts_quick_randomized(a, n, &seed); break;
        }
        if (memcmp(a, b, n * sizeof a[0]) != 0)
            fail("sort mismatch vs reference");
    }
    printf("fuzz sorts: %u iters, n 0..%d, all six vs reference\n",
           (unsigned)iters, N);
}

/* ---------------- strs: involution and symmetry -------------------------- */

static void fuzz_strs(size_t iters)
{
    enum { BUF = 256 };
    char s[BUF + 1], rep[BUF + 1], again[BUF + 1], mirror[2 * BUF + 2];
    size_t it, j, n, len;

    for (it = 0; it < iters; it++) {
        char from = (char)('a' + (f_next() % 4));
        char to = (char)('a' + (f_next() % 4));
        n = (size_t)(f_next() % (BUF + 1));
        for (j = 0; j < n; j++)
            s[j] = (char)('a' + (f_next() % 5));
        s[n] = '\0';
        strcpy(rep, s);
        strs_replace(rep, from, to);
        strcpy(again, rep);
        strs_replace(again, from, to);
        if (strcmp(again, rep) != 0)
            fail("replace not idempotent");
        strcpy(again, rep);
        strs_reverse(again);
        strs_reverse(again);
        if (strcmp(again, rep) != 0)
            fail("reverse involution broken");
        /* mirror = rep + reverse(rep) always reads the same both ways */
        len = strlen(rep);
        memcpy(mirror, rep, len);
        for (j = 0; j < len; j++)
            mirror[len + j] = rep[len - 1 - j];
        mirror[2 * len] = '\0';
        if (strs_is_palindrome(mirror) != 1)
            fail("mirrored string not recognized as palindrome");
    }
    printf("fuzz strs: %u iters, len 0..%d, replace/reverse/palindrome\n",
           (unsigned)iters, BUF);
}

/* ---------------- search: agreement on sorted data ------------------------ */

static void fuzz_search(size_t iters)
{
    enum { N = 256 };
    int a[N];
    size_t it, i, n;

    for (it = 0; it < iters; it++) {
        int key = (int)(f_next() % 100);
        n = (size_t)(f_next() % (N + 1));
        for (i = 0; i < n; i++)
            a[i] = (int)(f_next() % 100);
        ref_sort(a, n); /* binary search precondition */
        if ((search_binary(a, n, key) < 0) != (search_linear(a, n, key) < 0))
            fail("binary/linear found-mismatch");
        if ((search_binary_rec(a, n, key) < 0) != (search_binary(a, n, key) < 0))
            fail("binary rec/iterative mismatch");
        if (search_linear_rec(a, n, key) != search_linear(a, n, key))
            fail("linear rec/iterative mismatch");
    }
    printf("fuzz search: %u iters, n 0..%d, four variants cross-checked\n",
           (unsigned)iters, N);
}

int main(void)
{
    fuzz_luhn(100000);
    fuzz_reservoir(50000);
    fuzz_msort(5000);
    fuzz_slist(10000);
    fuzz_pfac(20000);
    fuzz_sorts(6000);
    fuzz_strs(20000);
    fuzz_search(20000);
    printf("%s: bounded fuzzing complete\n", failures ? "FAIL" : "OK");
    return failures ? 1 : 0;
}
