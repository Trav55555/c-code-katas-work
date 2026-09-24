#ifndef DLIST_H
#define DLIST_H

#include <stddef.h>

/* Doubly linked list of ints with explicit ownership (mirrors slist).
 *
 * Ownership: the list owns its nodes; dlist_destroy releases everything
 *   through one forward walk and accepts NULL.
 * Borrowing: dlist_get / dlist_get_from_end return borrowed pointers valid
 *   until the next dlist_append / dlist_remove_first on the same list, or
 *   dlist_destroy. The caller never frees them.
 * Failure: allocation failure in dlist_create/append leaves the list
 *   unchanged (NULL / -1). Other misuse is asserted.
 * dlist_get_from_end(0) is the tail element; both lookups are O(n)
 * (the reverse walk uses the back links). */
typedef struct DList DList;

DList *dlist_create(void);
void   dlist_destroy(DList *list);
int    dlist_append(DList *list, int value);
int    dlist_remove_first(DList *list, int value); /* 1 removed, 0 not found */
size_t dlist_length(const DList *list);
const int *dlist_get(const DList *list, size_t index);
const int *dlist_get_from_end(const DList *list, size_t index_from_end);

#endif