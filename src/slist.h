#ifndef SLIST_H
#define SLIST_H

#include <stddef.h>

/* Singly linked list of ints with explicit ownership.
 *
 * Ownership: the list owns its nodes. slist_destroy releases every node and
 *   the list itself. Callers never free node memory directly.
 * Borrowing: slist_get returns a borrowed pointer to the stored int. It stays
 *   valid until the next slist_append / slist_remove_first on the same list,
 *   or until slist_destroy. The caller must not free it.
 * Failure: allocation failure in slist_create/append leaves the list
 *   unchanged (create returns NULL; append returns -1). All other arguments
 *   are programmer errors and asserted. slist_destroy accepts NULL. */
typedef struct SList SList;

SList *slist_create(void);                      /* NULL on allocation failure */
void   slist_destroy(SList *list);              /* NULL-safe */
int    slist_append(SList *list, int value);    /* 0 ok, -1 alloc failure */
int    slist_remove_first(SList *list, int value); /* 1 removed, 0 not found */
size_t slist_length(const SList *list);
const int *slist_get(const SList *list, size_t index); /* NULL if out of range */

#endif
