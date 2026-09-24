/* Bounded fuzzing for the modules' input surfaces, run under ASan+UBSan.
 * Oracles: luhn returns only {0,1}; reservoir outputs are distinct and in
 * range; msort matches a reference insertion sort (differential); slist
 * matches an array model; pfac matches a reference factorization and
 * reconstructs its input by multiplication; dlist matches an array model in
 * both directions; bst matches a set model, including a walk whose visitor
 * reads the tree; num matches brute-force divisor, gcd, and primality
 * references; recmath matches iterative references and formatted digits;
 * euler_number_to_words keeps the snprintf prefix contract for every cap. Deterministic PRNG corpus.
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
#include "dlist.h"
#include "bst.h"
#include "num.h"
#include "recmath.h"
#include "euler.h"

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

/* ---------------- dlist: array model, forward and reverse ----------------- */

static void fuzz_dlist(size_t iters)
{
    enum { MODEL = 64, OPS = 128 };
    int model[MODEL];
    size_t it, op, nm, i, idx;

    for (it = 0; it < iters; it++) {
        DList *l = dlist_create();
        nm = 0;
        if (l == NULL)
            continue;
        for (op = 0; op < OPS; op++) {
            switch (f_next() % 3) {
            case 0:
                if (nm < MODEL) {
                    int v = (int)(f_next() % 8);
                    if (dlist_append(l, v) != 0)
                        fail("dlist append failed with memory available");
                    model[nm++] = v;
                }
                break;
            case 1: {
                int v = (int)(f_next() % 8), expect = 0;
                size_t m, found = 0;
                for (m = 0; m < nm; m++)
                    if (model[m] == v) {
                        found = m;
                        expect = 1;
                        break;
                    }
                if (dlist_remove_first(l, v) != expect)
                    fail("dlist remove_first mismatch");
                if (expect) {
                    for (i = found; i + 1 < nm; i++)
                        model[i] = model[i + 1];
                    nm--;
                }
                break;
            }
            default:
                idx = (size_t)(f_next() % (MODEL + 1));
                if (dlist_length(l) != nm)
                    fail("dlist length mismatch");
                if (idx >= nm) {
                    if (dlist_get(l, idx) != NULL || dlist_get_from_end(l, idx) != NULL)
                        fail("dlist get out of range");
                } else if (dlist_get(l, idx) == NULL ||
                           *dlist_get(l, idx) != model[idx] ||
                           dlist_get_from_end(l, idx) == NULL ||
                           *dlist_get_from_end(l, idx) != model[nm - 1 - idx]) {
                    fail("dlist get value mismatch");
                }
                break;
            }
        }
        dlist_destroy(l);
    }
    printf("fuzz dlist: %u iters, %u ops each, model-checked both directions\n",
           (unsigned)iters, (unsigned)OPS);
}

/* ---------------- bst: set model, walk order, reads during the walk ------ */

enum { BST_KEYS = 64 };

typedef struct {
    const BST *tree;
    int seen[BST_KEYS];
    size_t count;
    int reads_ok;
} BSTWalkLog;

static void bst_log_visit(int value, void *user)
{
    BSTWalkLog *log = user;
    if (log->count < BST_KEYS)
        log->seen[log->count] = value;
    log->count++;
    /* read the tree mid-walk: keys are even, so value + 1 is an absent
     * key that sorts between nodes (the case a threaded walk loops on) */
    if (!bst_contains(log->tree, value) || bst_contains(log->tree, value + 1))
        log->reads_ok = 0;
}

static void fuzz_bst(size_t iters)
{
    enum { OPS = 160 };
    int present[BST_KEYS];
    size_t it, op, k, nm, j;
    BSTWalkLog log;

    for (it = 0; it < iters; it++) {
        BST *t = bst_create();
        if (t == NULL)
            continue;
        memset(present, 0, sizeof present);
        nm = 0;
        for (op = 0; op < OPS; op++) {
            size_t slot = (size_t)(f_next() % BST_KEYS);
            int key = 2 * (int)slot;
            switch (f_next() % 4) {
            case 0:
                if (bst_insert(t, key) != (present[slot] ? 1 : 0))
                    fail("bst insert mismatch");
                if (!present[slot]) {
                    present[slot] = 1;
                    nm++;
                }
                break;
            case 1:
                if (bst_remove(t, key) != present[slot])
                    fail("bst remove mismatch");
                if (present[slot]) {
                    present[slot] = 0;
                    nm--;
                }
                break;
            case 2:
                if (bst_contains(t, key) != present[slot] || bst_size(t) != nm)
                    fail("bst contains/size mismatch");
                break;
            default:
                log.tree = t;
                log.count = 0;
                log.reads_ok = 1;
                bst_walk(t, bst_log_visit, &log);
                if (log.count != nm || !log.reads_ok)
                    fail("bst walk count or mid-walk read mismatch");
                for (k = 0, j = 0; k < BST_KEYS && j < log.count; k++)
                    if (present[k] && log.seen[j++] != 2 * (int)k)
                        fail("bst walk order mismatch");
                break;
            }
        }
        bst_destroy(t);
    }
    printf("fuzz bst: %u iters, %u ops each, set-model-checked with walks\n",
           (unsigned)iters, (unsigned)OPS);
}

/* ---------------- num: brute-force references ---------------------------- */

static void fuzz_num(size_t iters)
{
    enum { MAXN = 5000, CAP = 64 };
    uint64_t got[CAP], want[CAP];
    size_t it, count, nwant, cap;

    for (it = 0; it < iters; it++) {
        uint64_t a = f_next() % MAXN, b = f_next() % MAXN, d, g = 0;
        int r, prime;

        /* divisors of a, ascending, with snprintf-style capacity */
        nwant = 0;
        for (d = 1; d <= a; d++)
            if (a % d == 0 && nwant < CAP)
                want[nwant++] = d;
        cap = (size_t)(f_next() % (CAP + 1));
        r = num_factors(a, cap ? got : NULL, cap, &count);
        if (count != nwant || r != (nwant > cap))
            fail("num_factors count/status mismatch");
        else if (r == 0 && memcmp(got, want, nwant * sizeof got[0]) != 0)
            fail("num_factors values mismatch");

        /* common factors: divisors of both */
        nwant = 0;
        for (d = 1; d <= (a < b ? a : b); d++)
            if (a % d == 0 && b % d == 0 && nwant < CAP) {
                want[nwant++] = d;
                g = d;
            }
        if (a == 0 || b == 0)
            g = a > b ? a : b; /* gcd(0, x) = x; divisor lists differ there */
        if (num_gcd(a, b) != g || num_gcd_naive(a, b) != g)
            fail("gcd mismatch");
        if (a != 0 && b != 0) {
            r = num_common_factors(a, b, got, CAP, &count);
            if (r != 0 || count != nwant ||
                memcmp(got, want, nwant * sizeof got[0]) != 0)
                fail("num_common_factors mismatch");
        }

        prime = a >= 2;
        for (d = 2; d < a && prime; d++)
            if (a % d == 0)
                prime = 0;
        if (num_is_prime(a) != prime)
            fail("num_is_prime mismatch");
    }
    printf("fuzz num: %u iters, n < %d, brute-force references\n",
           (unsigned)iters, MAXN);
}

/* ---------------- recmath: iterative and formatted references ------------ */

static void fuzz_recmath(size_t iters)
{
    size_t it;
    for (it = 0; it < iters; it++) {
        int64_t n = (int64_t)f_next();
        unsigned k = (unsigned)(f_next() % 120), j;
        char digits[32];
        int want = 0, len, idx;
        uint64_t v, fa = 0, fb = 1, fact = 1, t;

        if (it == 0)
            n = INT64_MIN;
        len = snprintf(digits, sizeof digits, "%lld", (long long)n);
        for (idx = 0; idx < len; idx++)
            if (digits[idx] >= '0' && digits[idx] <= '9')
                want += digits[idx] - '0';
        if (recmath_sum_of_digits(n) != want)
            fail("sum_of_digits mismatch");

        for (j = 0; j < k && j < 94; j++) { /* fa = F(min(k, 94)) */
            t = fa + fb;
            fa = fb;
            fb = t;
        }
        if (k <= 93) {
            if (recmath_fibonacci(k, &v) != 0 || v != fa)
                fail("fibonacci mismatch");
        } else if (recmath_fibonacci(k, &v) != 1) {
            fail("fibonacci guard mismatch");
        }

        for (j = 2; j <= k && j <= 20; j++)
            fact *= j;
        if (k <= 20) {
            if (recmath_factorial(k, &v) != 0 || v != fact)
                fail("factorial mismatch");
        } else if (recmath_factorial(k, &v) != 1) {
            fail("factorial guard mismatch");
        }
    }
    printf("fuzz recmath: %u iters, digits/fibonacci/factorial references\n",
           (unsigned)iters);
}

/* ---------------- euler_number_to_words: prefix contract ------------------ */

static void fuzz_words(size_t iters)
{
    enum { BUF = 64 };
    char full[BUF], part[BUF + 1];
    size_t it, needed, part_needed, cap, wrote, i, letters;

    for (it = 0; it < iters; it++) {
        unsigned n = (unsigned)(f_next() % 10100); /* past the 9999 edge */
        cap = (size_t)(f_next() % (BUF + 1));
        euler_number_to_words(n, full, sizeof full, &needed);
        if (n > 9999 ? (needed != 0 || full[0] != '\0') : strlen(full) != needed)
            fail("number_to_words full length mismatch");
        memset(part, '#', sizeof part);
        wrote = euler_number_to_words(n, cap ? part : NULL, cap, &part_needed);
        if (part_needed != needed)
            fail("number_to_words needed depends on cap");
        if (cap == 0) {
            if (wrote != 0 || part[0] != '#')
                fail("number_to_words wrote with cap 0");
        } else if (wrote != (needed < cap ? needed : cap - 1) ||
                   part[wrote] != '\0' || strncmp(part, full, wrote) != 0 ||
                   part[cap] != '#') {
            fail("number_to_words prefix contract violated");
        }
        letters = 0;
        for (i = 0; i < needed; i++)
            if (full[i] != ' ' && full[i] != '-')
                letters++;
        if (euler_number_name_letters(n) != letters)
            fail("number_name_letters disagrees with the words");
    }
    printf("fuzz words: %u iters, n 0..10099, cap 0..%d, prefix contract\n",
           (unsigned)iters, BUF);
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
    fuzz_dlist(10000);
    fuzz_bst(10000);
    fuzz_num(3000);
    fuzz_recmath(50000);
    fuzz_words(50000);
    printf("%s: bounded fuzzing complete\n", failures ? "FAIL" : "OK");
    return failures ? 1 : 0;
}
