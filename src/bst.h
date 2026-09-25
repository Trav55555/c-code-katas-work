#ifndef BST_H
#define BST_H

#include <stddef.h>

/* Binary search tree of ints (no duplicates) with explicit ownership.
 *
 * Ownership: the tree owns its nodes; bst_destroy releases every node
 *   without recursion or a side stack (rotation-based teardown, O(1)
 *   auxiliary space) and accepts NULL.
 * Failure: bst_insert allocates before mutating, so -1 leaves the tree
 *   unchanged. Duplicate insert returns 1 and is a no-op.
 * Walk: in-order (ascending) over parent links, O(1) auxiliary space. The
 *   walk does not modify the tree, so `visit` (non-NULL) may call the
 *   read-only functions on it; it must not insert, remove, or destroy.
 * Cost: no balancing -- operations are O(height), worst case O(n) on
 *   sorted insert order. Removal handles the two-child case by copying
 *   the in-order successor and freeing its node. */
typedef struct BST BST;
typedef void (*BSTVisitFunc)(int value, void *user);

BST *bst_create(void);
void  bst_destroy(BST *tree);
int   bst_insert(BST *tree, int value);   /* 0 inserted, 1 duplicate, -1 alloc */
int   bst_contains(const BST *tree, int value);
int   bst_remove(BST *tree, int value);   /* 1 removed, 0 absent */
size_t bst_size(const BST *tree);
void  bst_walk(const BST *tree, BSTVisitFunc visit, void *user); /* ascending */

#endif