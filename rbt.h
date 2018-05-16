#ifndef __RBT_H__
#define __RBT_H__

#define RED     0
#define BLACK   1

typedef unsigned int rb_key_t;

// Red-Black Node structure
struct rb_node_s {
    struct rb_node_s *parent;
    struct rb_node_s *left;
    struct rb_node_s *right;

    rb_key_t key;
    void *value;
    int   color; // 0(RED) or 1(BLACK)
};

// Red-Black Tree structure
struct rb_tree_s {
    struct rb_node_s *root;
};

typedef struct rb_node_s rb_node_t;
typedef struct rb_tree_s rb_tree_t;


// Red-Black Tree implementation
rb_tree_t  *rb_create();
rb_node_t  *rb_create_node();
int         rb_insert(rb_tree_t *tree, rb_key_t ikey, void *value);
void        rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node);
int         rb_find(rb_tree_t *tree, rb_key_t skey, rb_node_t **node);

#endif
