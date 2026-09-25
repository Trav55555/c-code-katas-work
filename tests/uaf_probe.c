/* Negative test: a borrowed pointer used after slist_remove_first must be
 * caught by ASan. This binary is EXPECTED to abort under the sanitizer; the
 * make target turns that into a pass. Without ASan it prints NOT CAUGHT. */
#include <stdio.h>

#include "slist.h"

int main(void)
{
    SList *l;
    const int *p;
    int v;

    l = slist_create();
    if (l == NULL)
        return 2;
    if (slist_append(l, 42) != 0)
        return 2;
    p = slist_get(l, 0);       /* borrowed, valid until the next mutation */
    slist_remove_first(l, 42); /* contract: p is now invalid */
    v = *p;                    /* deliberate contract violation */
    slist_destroy(l);
    printf("NOT CAUGHT %d\n", v);
    return 0;
}
