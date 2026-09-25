#include "dlist.h"

#include <assert.h>
#include <stdlib.h>

typedef struct DListNode {
    int value;
    struct DListNode *prev;
    struct DListNode *next;
} DListNode;

struct DList {
    DListNode *head;
    DListNode *tail;
    size_t length;
};

DList *dlist_create(void)
{
    DList *list = malloc(sizeof *list);
    if (list == NULL)
        return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

void dlist_destroy(DList *list)
{
    DListNode *n;
    if (list == NULL)
        return;
    n = list->head;
    while (n != NULL) {
        DListNode *next = n->next;
        free(n);
        n = next;
    }
    free(list);
}

int dlist_append(DList *list, int value)
{
    DListNode *n;
    assert(list != NULL);
    n = malloc(sizeof *n); /* sole acquisition: nothing mutated before it */
    if (n == NULL)
        return -1;         /* list unchanged */
    n->value = value;
    n->prev = list->tail;
    n->next = NULL;
    if (list->tail == NULL)
        list->head = n;
    else
        list->tail->next = n;
    list->tail = n;
    list->length++;
    return 0;
}

int dlist_remove_first(DList *list, int value)
{
    DListNode *n;
    assert(list != NULL);
    for (n = list->head; n != NULL; n = n->next)
        if (n->value == value) {
            if (n->prev == NULL)
                list->head = n->next;
            else
                n->prev->next = n->next;
            if (n->next == NULL)
                list->tail = n->prev;
            else
                n->next->prev = n->prev;
            free(n);
            list->length--;
            return 1;
        }
    return 0;
}

size_t dlist_length(const DList *list)
{
    assert(list != NULL);
    return list->length;
}

const int *dlist_get(const DList *list, size_t index)
{
    DListNode *n;
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

const int *dlist_get_from_end(const DList *list, size_t index_from_end)
{
    DListNode *n;
    assert(list != NULL);
    if (index_from_end >= list->length)
        return NULL;
    n = list->tail;
    while (index_from_end > 0) {
        n = n->prev;
        index_from_end--;
    }
    return &n->value;
}