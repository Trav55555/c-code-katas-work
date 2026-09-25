#include "slist.h"

#include <assert.h>
#include <stdlib.h>

typedef struct SListNode {
    int value;
    struct SListNode *next;
} SListNode;

struct SList {
    SListNode *head;
    SListNode *tail;
    size_t length; /* bounded by allocatable nodes; cannot reach SIZE_MAX */
};

SList *slist_create(void)
{
    SList *list = malloc(sizeof *list);
    if (list == NULL)
        return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

void slist_destroy(SList *list)
{
    SListNode *n;
    if (list == NULL)
        return;
    n = list->head; /* single cleanup path, forward order (no back pointers) */
    while (n != NULL) {
        SListNode *next = n->next;
        free(n);
        n = next;
    }
    free(list);
}

int slist_append(SList *list, int value)
{
    SListNode *n;
    assert(list != NULL);
    n = malloc(sizeof *n); /* the only acquisition; nothing mutated before it */
    if (n == NULL)
        return -1;         /* list unchanged: outputs preserved on failure */
    n->value = value;
    n->next = NULL;
    if (list->tail == NULL)
        list->head = n;
    else
        list->tail->next = n;
    list->tail = n;
    list->length++;
    return 0;
}

int slist_remove_first(SList *list, int value)
{
    SListNode *n, *prev = NULL;
    assert(list != NULL);
    n = list->head;
    while (n != NULL) {
        if (n->value == value) {
            if (prev == NULL)
                list->head = n->next;
            else
                prev->next = n->next;
            if (list->tail == n)
                list->tail = prev;
            free(n);
            list->length--;
            return 1;
        }
        prev = n;
        n = n->next;
    }
    return 0;
}

size_t slist_length(const SList *list)
{
    assert(list != NULL);
    return list->length;
}

const int *slist_get(const SList *list, size_t index)
{
    SListNode *n;
    assert(list != NULL);
    if (index >= list->length)
        return NULL;
    n = list->head;
    while (index > 0) {
        n = n->next;
        index--;
    }
    return &n->value;
}
