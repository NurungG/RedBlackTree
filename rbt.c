#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbt.h"

#define RED     0
#define BLACK   1

/* Create Red-Black Tree */
rb_tree_t *rb_create() {
    rb_tree_t *tree = NULL;
    
    if ((tree = malloc(sizeof(rb_tree_t))) == NULL) {
        return NULL;
    }

    tree->root = NULL;

    return tree;
}

/* Create Red-Black Node */
rb_node_t *rb_create_node() {
    rb_node_t *node = NULL;

    if ((node = malloc(sizeof(rb_node_t))) == NULL) {
        return NULL;
    }

    memset(node, 0, sizeof(rb_node_t));

/*  node->parent = NULL;
    node->left   = NULL;
    node->right  = NULL;

    node->key    = 0;
    node->value  = NULL;
    node->color  = RED;  // default color is RED    */

    return node;
}

/* Insert {key, value} pair to tree */
int rb_insert(rb_tree_t *tree, rb_key_t ikey, void *value) {
    rb_node_t *root;
    rb_node_t *vacant;
    rb_node_t *parent;
    rb_node_t *uncle;

    if (tree->root == NULL) {
        // Case of empty
        root = rb_create_node();

        root->key   = ikey;
        root->value = value;
        root->color = BLACK;

        tree->root = root;
        
    } else {
        // Common case
        vacant = tree->root;
        parent = NULL;
        
        while (vacant != NULL) {
            parent = vacant;
            
            if (ikey < vacant->key) {
                // Go left
                vacant = vacant->left;

            } else if (ikey > vacant->key) {
                // Go right
                vacant = vacant->right;

            } else {
                // Already exists
                return -1;
            }
        }

        // Create node on the vacant
        vacant = rb_create_node();

        vacant->parent = parent;
        vacant->key    = ikey;
        vacant->value  = value;

        // Setup child pointer of parent
        if (vacant->key < parent->key) {
            // Left child
            parent->left = vacant;

        } else {
            // Right child
            parent->right = vacant;
        }

        // Load balancing
        if (parent->color == RED) {
            // Double red occur
            rb_remedy_double_red(tree, vacant);
        }
    }
    return 0;
}

/* Get sibling of the node */
static rb_node_t *get_sibling(rb_node_t *node) {
    rb_node_t *sibling;

    if (node->parent == NULL) { // case of root
        return NULL;
    }
    
    if (node->parent->left == node) {
        sibling = node->parent->right;
    } else {
        sibling = node->parent->left;
    }

    return sibling;
}

/* Recolor two RED nodes to BLACK, and a parent of them to RED */
static void recoloring(rb_tree_t *tree, rb_node_t *node) {
    rb_node_t *parent;
    rb_node_t *sibling;

    parent  = node->parent;
    sibling = get_sibling(node);

    node->color    = BLACK;
    sibling->color = BLACK;
    
    if (parent != tree->root) {
        parent->color = RED;
        
        // Treat propagation
        if (parent->parent->color == RED) {
            // Double red propagates
            rb_remedy_double_red(tree, parent); 
        }
    }

    // If parent is root vertex, there is no propagation
    // And root vertex still remain as BLACK node
}

/* Setup pointer information before restructuring */
static void restructuring_setup(
    rb_node_t *node, rb_node_t **p, rb_node_t **l, rb_node_t **r,
    rb_node_t **lrc, rb_node_t **rlc) {

    rb_node_t *parent = node->parent;
    rb_node_t *grand  = parent->parent;

    if (grand->left == parent) {
        if (parent->left == node) {
            // left-left
            *l = node;               /*     BLACK */
            *r = grand;              /*     /     */
            *p = parent;             /*   RED     */
            *lrc = node->right;      /*   /       */
            *rlc = parent->right;    /* RED       */

        } else {
            // left-right
            *l = parent;             /*   BLACK   */
            *r = grand;              /*   /       */
            *p = node;               /* RED       */
            *lrc = node->left;       /*   \       */
            *rlc = node->right;      /*   RED     */
        }

    } else {
        if (parent->left == node) {
            // right-left
            *l = grand;              /*  BLACK    */
            *r = parent;             /*      \    */
            *p = node;               /*      RED  */
            *lrc = node->left;       /*      /    */
            *rlc = node->right;      /*     RED   */
            
        } else {
            // right-right
            *l = grand;              /* BLACK     */
            *r = node;               /*     \     */
            *p = parent;             /*     RED   */
            *lrc = parent->left;     /*       \   */
            *rlc = node->left;       /*       RED */
        }
    }
}

/* Restructure the sub-tree of tree */
static void restructuring(rb_tree_t *tree, rb_node_t *node) {
    rb_node_t *parent;
    rb_node_t *left;
    rb_node_t *right;
    
    rb_node_t *left_right_child;
    rb_node_t *right_left_child;

    rb_node_t *grand = node->parent->parent;

    // Setup pointers (get each position to be restructured)
    restructuring_setup(
        node, &parent, &left, &right, &left_right_child, &right_left_child);

    // Change color
    parent->color = BLACK;
    left->color   = RED;
    right->color  = RED;

    // Renew child pointers
    parent->left  = left;
    parent->right = right;

    left->right = left_right_child;
    right->left = right_left_child;

    // Renew parents
    parent->parent = grand->parent;
    left->parent   = parent;
    right->parent  = parent;

    if (left_right_child != NULL) left_right_child->parent = left;
    if (right_left_child != NULL) right_left_child->parent = right;

    // Connect with ancestor
    if (parent->parent == NULL) {
        // Case of root
        tree->root = parent;

    } else {
        // Common case
        if (parent->parent->left == grand) {
            parent->parent->left  = parent;
        } else {
            parent->parent->right = parent;
        }
    }

    // On restructuring, it doesn't propagate to upper layer
}

/* Remedy the double red situation by appropriate solution */
void rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node) {
    rb_node_t *parent = node->parent;
    rb_node_t *uncle  = get_sibling(parent);

    // Double red situation guarantees the node has at least height of 3
    // So there is no need to doubt that the grand parent is NULL

    if (uncle != NULL && uncle->color == RED) { // recoloring
        recoloring(tree, parent);
    } else { // restructuring
        restructuring(tree, node);
    }
}

/* Find the node */
int rb_find(rb_tree_t *tree, rb_key_t skey, rb_node_t **found) {
    int depth = 0;
    rb_node_t *node = tree->root;

    // Search
    while (node != NULL) {
        if (skey== node->key) { // find!
            break;
        } else if (skey < node->key) { // go left
            node = node->left;
        } else { // go right
            node = node->right;
        }
        ++depth;
    }

    // Fail to find
    if (node == NULL) {
        depth = -1;
    }

    // Save the node pointer
    if (found != NULL) {
        *found = node;
    }

    return depth;
}
