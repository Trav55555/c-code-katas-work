/* Allocation-failure injection tests: the failure paths of the modules'
 * contracts ("returns NULL/-1 and leaves the list/array UNMODIFIED").
 * Built with -Dmalloc=fault_malloc over the module sources. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fault_alloc.h"
#include "slist.h"
#include "msort.h"

static int failures = 0;
#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); \
            failures++; \
        } \
    } while (0)

static void test_slist_failures(void)
{
    SList *l;
    int before[4] = { 4, 3, 2, 1 };
    int snapshot[4];

    /* create: first allocation fails -> NULL, nothing to clean up */
    fault_alloc_countdown = 0;
    CHECK(slist_create() == NULL);

    /* create succeeds, append fails at the node allocation */
    fault_alloc_countdown = 0;
    l = slist_create();
    CHECK(l == NULL); /* countdown still 0: create itself fails first */
    fault_alloc_countdown = 1;
    l = slist_create(); /* one pass-through then armed */
    CHECK(l != NULL);
    if (l == NULL)
        return;
    CHECK(slist_append(l, 7) == -1);         /* armed: node malloc fails */
    CHECK(slist_length(l) == 0);             /* unchanged */
    fault_alloc_countdown = -1;              /* re-arm to pass */
    CHECK(slist_append(l, 7) == 0);          /* recovery works */
    CHECK(slist_length(l) == 1 && *slist_get(l, 0) == 7);

    /* append failure mid-list preserves all prior outputs */
    fault_alloc_countdown = 3;
    CHECK(slist_append(l, 8) == 0);          /* 2 pass-throughs left */
    CHECK(slist_append(l, 9) == 0);          /* 1 left */
    fault_alloc_countdown = 0;
    CHECK(slist_append(l, 10) == -1);        /* fails */
    CHECK(slist_length(l) == 3);
    CHECK(*slist_get(l, 0) == 7 && *slist_get(l, 1) == 8 && *slist_get(l, 2) == 9);
    (void)before;
    (void)snapshot;
    slist_destroy(l);                        /* full cleanup after failures */
}

static void test_msort_failures(void)
{
    int a[5] = { 5, 4, 3, 2, 1 };
    int orig[5] = { 5, 4, 3, 2, 1 };
    int one[1] = { 9 };

    /* scratch allocation fails -> -1, array unmodified (contract) */
    fault_alloc_countdown = 0;
    CHECK(msort_sort(a, 5) == -1);
    CHECK(memcmp(a, orig, sizeof a) == 0);

    /* no allocation attempted for n < 2 even while armed to fail */
    fault_alloc_countdown = 0;
    CHECK(msort_sort(NULL, 0) == 0);
    CHECK(msort_sort(one, 1) == 0 && one[0] == 9);
    fault_alloc_countdown = -1;

    /* and the recovery path still sorts */
    CHECK(msort_sort(a, 5) == 0 && a[0] == 1 && a[4] == 5);
}

int main(void)
{
    test_slist_failures();
    test_msort_failures();
    printf("%s: %d failure(s)\n", failures ? "FAIL" : "OK", failures);
    return failures ? 1 : 0;
}
