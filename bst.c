#include "bst.h"

#include <assert.h>
#include <stdlib.h>

typedef struct BSTNode {
    int value;
    struct BSTNode *left;
    struct BSTNode *right;
    struct BSTNode *parent; /* NULL at the root */
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
     * nodes on the right. No recursion, no side stack. Parent links go
     * stale during the rotations, which is fine: nothing reads them. */
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
    BSTNode *n, **link, *parent = NULL;
    assert(tree != NULL);
    link = &tree->root;
    while (*link != NULL) {
        parent = *link;
        if (value == parent->value)
            return 1; /* duplicate: no-op */
        link = (value < parent->value) ? &parent->left : &parent->right;
    }
    n = malloc(sizeof *n); /* sole acquisition, before any mutation */
    if (n == NULL)
        return -1;         /* tree unchanged */
    n->value = value;
    n->left = NULL;
    n->right = NULL;
    n->parent = parent;
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
        if (succ->right != NULL)
            succ->right->parent = succ->parent;
        free(succ);
    } else {
        BSTNode *child = (n->left != NULL) ? n->left : n->right;
        *link = child;
        if (child != NULL)
            child->parent = n->parent;
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

static const BSTNode *leftmost(const BSTNode *n)
{
    while (n->left != NULL)
        n = n->left;
    return n;
}

/* In-order walk over parent links: O(1) auxiliary space, and the tree is
 * never modified, so `visit` may read it. */
void bst_walk(const BST *tree, BSTVisitFunc visit, void *user)
{
    const BSTNode *n;
    assert(tree != NULL);
    assert(visit != NULL);
    if (tree->root == NULL)
        return;
    n = leftmost(tree->root);
    while (n != NULL) {
        visit(n->value, user);
        if (n->right != NULL) {
            n = leftmost(n->right);
        } else {
            /* climb until we leave a left subtree */
            while (n->parent != NULL && n == n->parent->right)
                n = n->parent;
            n = n->parent;
        }
    }
}
