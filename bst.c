#include "bst.h"

#include <assert.h>
#include <stdlib.h>

typedef struct BSTNode {
    int value;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

struct BST {
    BSTNode *root;
    size_t size;
};

BST *bst_create(void)
{
    BST *tree = malloc(sizeof *tree);
    if (tree == NULL)
        return NULL;
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

void bst_destroy(BST *tree)
{
    BSTNode *n;
    if (tree == NULL)
        return;
    /* Rotation-based teardown: always descend the left spine, freeing
     * nodes on the right. No recursion, no side stack. */
    n = tree->root;
    while (n != NULL) {
        if (n->left != NULL) {
            BSTNode *l = n->left;
            n->left = l->right;
            l->right = n;
            n = l;
        } else {
            BSTNode *r = n->right;
            free(n);
            n = r;
        }
    }
    free(tree);
}

int bst_insert(BST *tree, int value)
{
    BSTNode *n, **link;
    assert(tree != NULL);
    link = &tree->root;
    while (*link != NULL) {
        n = *link;
        if (value == n->value)
            return 1; /* duplicate: no-op */
        link = (value < n->value) ? &n->left : &n->right;
    }
    n = malloc(sizeof *n); /* sole acquisition, before any mutation */
    if (n == NULL)
        return -1;         /* tree unchanged */
    n->value = value;
    n->left = NULL;
    n->right = NULL;
    *link = n;
    tree->size++;
    return 0;
}

int bst_contains(const BST *tree, int value)
{
    BSTNode *n;
    assert(tree != NULL);
    n = tree->root;
    while (n != NULL) {
        if (value == n->value)
            return 1;
        n = (value < n->value) ? n->left : n->right;
    }
    return 0;
}

int bst_remove(BST *tree, int value)
{
    BSTNode *n, **link, *victim;
    assert(tree != NULL);
    link = &tree->root;
    n = tree->root;
    while (n != NULL && value != n->value) {
        link = (value < n->value) ? &n->left : &n->right;
        n = *link;
    }
    if (n == NULL)
        return 0;
    victim = n;
    if (n->left != NULL && n->right != NULL) {
        /* two children: copy the in-order successor, then free its node */
        BSTNode **succ_link = &n->right;
        BSTNode *succ = n->right;
        while (succ->left != NULL) {
            succ_link = &succ->left;
            succ = succ->left;
        }
        n->value = succ->value;
        *succ_link = succ->right;
        free(succ);
    } else {
        *link = (n->left != NULL) ? n->left : n->right;
        free(victim);
    }
    tree->size--;
    return 1;
}

size_t bst_size(const BST *tree)
{
    assert(tree != NULL);
    return tree->size;
}

/* Threaded (Morris) in-order traversal: O(1) auxiliary space, tree
 * restored before returning. */
void bst_walk(BST *tree, BSTVisitFunc visit, void *user)
{
    BSTNode *n;
    assert(tree != NULL);
    assert(visit != NULL);
    n = tree->root;
    while (n != NULL) {
        if (n->left == NULL) {
            visit(n->value, user);
            n = n->right;
        } else {
            BSTNode *pred = n->left;
            while (pred->right != NULL && pred->right != n)
                pred = pred->right;
            if (pred->right == NULL) {
                pred->right = n; /* thread */
                n = n->left;
            } else {
                pred->right = NULL; /* unthread */
                visit(n->value, user);
                n = n->right;
            }
        }
    }
}