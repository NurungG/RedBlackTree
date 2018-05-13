/* Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Defines */
#define RED     0
#define BLACK   1

#define DOWN    0
#define UP      1


/* Structures */

// Information of each account log
struct log_s {
    int updown;
    int amount;
};

// Log node structure
struct log_node_s {
    struct log_s value;
    struct log_node_s *next;
};

// Log list structure
struct log_list_s {
    struct log_node_s *head;
};

// Information of each member
struct member_s {
    int  id;
    char name[21];
    char phone[12];
    int  x, y;
    int  level;
    int  money;
    struct log_list_s *log;
};

// Red-Black Node structure
struct rb_node_s {
    struct rb_node_s *parent;
    struct rb_node_s *left;
    struct rb_node_s *right;
    struct member_s  value;
    int color;
};

// Red-Black Tree structure
struct rb_tree_s {
    struct rb_node_s *root;
};

typedef struct log_s log_t;
typedef struct log_node_s log_node_t;
typedef struct log_list_s log_list_t;

typedef struct member_s member_t;
typedef struct rb_node_s rb_node_t;
typedef struct rb_tree_s rb_tree_t;


/* Declare functions */
log_list_t *log_create();
log_node_t *log_create_node();
void        log_insert(log_list_t *list, log_t value);

rb_tree_t      *rb_create();
rb_node_t      *rb_create_node();
unsigned int    rb_insert(rb_tree_t *tree, member_t value);
int             rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node);
int             rb_recoloring(rb_tree_t *tree, rb_node_t *node);
int             rb_restructuring(rb_tree_t *tree, rb_node_t *node);
int             rb_treaverse_node_dfs(rb_node_t *node, int depth);
void            rb_treaverse_dfs(rb_tree_t *tree);
int             rb_find(rb_tree_t *tree, int id, rb_node_t **node);

int  execute_operation(rb_tree_t *tree, char op);
void op_join_member(rb_tree_t *tree);
void op_print_info(rb_tree_t *tree);
void op_add_cash(rb_tree_t *tree);
void op_find_top_five(rb_tree_t *tree);
void op_print_log(rb_tree_t *tree);
void op_buy_area(rb_tree_t *tree);

/* Global variables */
int area_price[1001][1001];
int area_owner[1001][1001];

/* Main function */
int main() {
    char filename[1024];
    FILE *input;
    
    rb_tree_t *tree;
    char op;

    memset(area_owner, -1, 1001 * 1001 * sizeof(int));

    // File in
    fputs("입력파일의 이름을 입력하세요 : ", stdout);
    fgets(filename, 1024, stdin);

    filename[strlen(filename)-1] = 0;
    input = fopen(filename, "r");
    if (input != NULL) {
        tree = rb_create();

        while (1) {
            member_t new_member;
            int ret;

            ret = fscanf(input, "%d %s %s %d %d %d %d",
                    &new_member.id, new_member.name, new_member.phone,
                    &new_member.x, &new_member.y, &new_member.level, &new_member.money);
            
            if (ret < 0) break;
            
            rb_insert(tree, new_member);
            area_owner[new_member.x][new_member.y] = new_member.id;
        }
        fclose(input);
    } // file input end 

    //puts("file setup finished");
    
    while (1) {
        scanf("%c", &op);
        if (execute_operation(tree, op) == 0) {
            break;
        }
    }

    return 0;
}

/* Function implementation */
log_list_t *log_create() {
    log_list_t *list = NULL;
    
    if ((list = malloc(sizeof(log_list_t))) == NULL) {
        return NULL;
    }

    list->head = NULL;

    return list;
}

log_node_t *log_create_node() {
    log_node_t *node = NULL;

    if ((node = malloc(sizeof(log_node_t))) == NULL) {
        return NULL;
    }

    node->next = NULL;
    // memset(&node->value, 0, sizeof(log_t));

    return node;
}

void log_insert(log_list_t *list, log_t value) {
    log_node_t *node;

    node = log_create_node();
    node->value = value;

    node->next = list->head;
    list->head = node;
}

rb_tree_t *rb_create() {
    rb_tree_t *tree = NULL;
    
    if ((tree = malloc(sizeof(rb_tree_t))) == NULL) {
        return NULL;
    }

    tree->root = NULL;

    return tree;
}

rb_node_t *rb_create_node() {
    rb_node_t *node = NULL;

    if ((node = malloc(sizeof(rb_node_t))) == NULL) {
        return NULL;
    }

    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    //memset(&node->value, 0, sizeof(member_t));
    node->color = RED;

    return node;
}

rb_node_t *get_sibling(rb_node_t *node) {
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

unsigned int rb_insert(rb_tree_t *tree, member_t value) {
    rb_node_t *root;
    rb_node_t *vacant;
    rb_node_t *parent;
    rb_node_t *uncle;

    unsigned int ret = 0x80000000;   // represents depth on least bits, and result on a most bit

    if (tree->root == NULL) { // case of empty
        root = rb_create_node();
        root->value = value;
        root->value.log = log_create();
        root->color = BLACK;
        
        tree->root = root;
        
    } else {
        vacant = tree->root;
        parent = NULL;
        
        while (vacant != NULL) {
            parent = vacant;

            if (value.id < vacant->value.id) { // go left
                vacant = vacant->left;

            } else if (value.id > vacant->value.id) { // go right 
                vacant = vacant->right;

            } else { // already exists
                ret &= 0x7fffffff;
                break;
            }

            ++ret;
        }

        if ((ret & 0x80000000) == 0) {
            return ret;
        }

        vacant = rb_create_node();
        vacant->parent = parent;
        vacant->value  = value;
        vacant->value.log = log_create();

        if (vacant->value.id < parent->value.id) {
            parent->left = vacant;
        } else {
            parent->right = vacant;
        }

        uncle = get_sibling(parent);
        if (parent->color == RED) { // double red
            if (uncle != NULL && uncle->color == RED) { // recoloring
                if (rb_recoloring(tree, parent) != 0) {
                    --ret;
                }
            } else { // restructuring
                ret -= rb_restructuring(tree, vacant);
            }
        } 
    }

    return ret;
}

int rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node) {
    int ret;
    rb_node_t *parent;
    rb_node_t *uncle;

    // Double red situation guarantees the node has at least height of 3
    // So there is no need to doubt that the grand parent is NULL

    parent = node->parent;
    uncle  = get_sibling(parent);

    if (uncle != NULL && uncle->color == RED) { // recoloring
        ret = rb_recoloring(tree, parent);
    } else { // restructuring
        ret = rb_restructuring(tree, node);
    }

    return ret;
}
    
int rb_recoloring(rb_tree_t *tree, rb_node_t *node) {
    int ret = 0;
    rb_node_t *parent;
    rb_node_t *sibling;

    parent  = node->parent;
    sibling = get_sibling(node);

    node->color    = BLACK;
    sibling->color = BLACK;
    
    if (parent != tree->root) {
        parent->color = RED;

        if (parent->parent->color == RED) { // propagation
            ret = rb_remedy_double_red(tree, parent);
        }
    }

    // If parent is root vertex, there is no propagation
    // And root vertex still remain as BLACK node

    return ret;
}

int restructuring_setup(
    rb_node_t *node, rb_node_t **p, rb_node_t **l, rb_node_t **r,
    rb_node_t **lrc, rb_node_t **rlc) {

    rb_node_t *parent;
    rb_node_t *grand;

    parent = node->parent;
    grand  = parent->parent;
    
    if (grand->left == parent) {
        if (parent->left == node) { // left-left
            *l = node;               /*     BLACK */
            *r = grand;              /*     /     */
            *p = parent;             /*   RED     */
            *lrc = node->right;      /*   /       */
            *rlc = parent->right;    /* RED       */
            return 1;

        } else { // left-right
            *l = parent;             /*   BLACK   */
            *r = grand;              /*   /       */
            *p = node;               /* RED       */
            *lrc = node->left;       /*   \       */
            *rlc = node->right;      /*   RED     */
            return 2;
        }

    } else {
        if (parent->left == node) { // right-left
            *l = grand;              /*  BLACK    */
            *r = parent;             /*      \    */
            *p = node;               /*      RED  */
            *lrc = node->left;       /*      /    */
            *rlc = node->right;      /*     RED   */
            return 2;
            
        } else { // right-right
            *l = grand;              /* BLACK     */
            *r = node;               /*     \     */
            *p = parent;             /*     RED   */
            *lrc = parent->left;     /*       \   */
            *rlc = node->left;       /*       RED */
            return 1;
        }
    }
}

int rb_restructuring(rb_tree_t *tree, rb_node_t *node) {
    int ret;
    rb_node_t *parent;
    rb_node_t *left;
    rb_node_t *right;
    
    rb_node_t *left_right_child;
    rb_node_t *right_left_child;

    rb_node_t *grand = node->parent->parent;

    ret = restructuring_setup(node, &parent, &left, &right, &left_right_child, &right_left_child);

    // Change color
    parent->color = BLACK;
    left->color   = RED;
    right->color  = RED;

    // Renew children of parent
    parent->left  = left;
    parent->right = right;

    // Renew parent pointers
    parent->parent = grand->parent;
    left->parent   = parent;
    right->parent  = parent;

    // Renew child pointers
    left->right = left_right_child;
    right->left = right_left_child;

    // Renew parent of grand child
    if (left_right_child != NULL) left_right_child->parent = left;
    if (right_left_child != NULL) right_left_child->parent = right;

    if (parent->parent == NULL) { // case of root
        tree->root = parent;
    } else {
        if (parent->parent->left == grand) {
            parent->parent->left  = parent;
        } else {
            parent->parent->right = parent;
        }
    }

    // On restructuring, it doesn't propagate to upper layer

    return ret;
}

int rb_treaverse_node_dfs(rb_node_t *node, int depth) {

}

void rb_treaverse_dfs(rb_tree_t *tree) {

}

int rb_find(rb_tree_t *tree, int id, rb_node_t **found) {
    int depth = 0;
    *found = tree->root;
    
    while (*found != NULL) {
        if (id == (*found)->value.id) { // get
            break;
        } else if (id < (*found)->value.id) { // go left
            *found = (*found)->left;
        } else { // go right
            *found = (*found)->right;
        }
        ++depth;
    }

    if (*found == NULL) {
        depth = -1;
    }

    return depth;
}


int execute_operation(rb_tree_t *tree, char op) {
    
    switch (op) {
    case 'I' :
        op_join_member(tree);
        break;
    case 'P' :
        op_print_info(tree);
        break;
    case 'A' :
        op_add_cash(tree);
        break;
    case 'F' :
        op_find_top_five(tree);
        break;
    case 'R' :
        op_print_log(tree);
        break;
    case 'B' :
        op_buy_area(tree);
        break;
    case '\n':
        break;
    case 'Q' :
        return 0;
    default :
        printf("Invalid operation %c\n", op);
    }

    return 1;
}

void op_join_member(rb_tree_t *tree) {
    member_t new_member;
    unsigned int ret;
    int depth, approval;

    rb_node_t *test;
    int tdepth;
    
    scanf("%d %s %s %d %d",
        &new_member.id, new_member.name, new_member.phone, &new_member.x, &new_member.y);
//    printf("Query :: I %d %s %s %d %d\n",
//        new_member.id, new_member.name, new_member.phone, new_member.x, new_member.y); // nurungg
    new_member.level = 0;
    new_member.money = 0;

    ret = rb_insert(tree, new_member); 
    
    // test
//    test = tree->root;
//    tdepth = 0;
//    while (test->value.id != new_member.id) {
//        if (new_member.id < test->value.id) {
//            test = test->left;
//        } else {
//            test = test->right;
//        }
//        tdepth++;
//    }
    // end

    depth = ret & 0x7fffffff;

    if ((ret & 0x80000000) == 0) {
        approval = 0;
    } else {
        approval = 1;
        
        if (area_owner[new_member.x][new_member.y] == -1) {
            area_owner[new_member.x][new_member.y] = new_member.id;
        }
    }

    printf("%d %d\n", depth, approval);
}

void op_print_info(rb_tree_t *tree) {
    int id, depth;
    rb_node_t *node;
    member_t member;
    
    scanf("%d", &id);
    //printf("Query :: P %d\n", id); // nurungg

    if ((depth = rb_find(tree, id, &node)) == -1) {
        puts("Not found!");
    } else {            
        member = node->value;
        printf("%s %s %d %d %d\n",
            member.name, member.phone, member.level, member.money, depth);
    }
}

void set_level(member_t *member) {
    if (member->money < 30000) {
        member->level = 0;

    } else if (member->money < 50000) {
        member->level = 1;

    } else if (member->money < 100000) {
        member->level = 2;

    } else {
        member->level = 3;
    } 
}       

void op_add_cash(rb_tree_t *tree) {
    int id, amount, depth;
    rb_node_t *node;
    log_t log;

    scanf("%d %d", &id, &amount);
    //printf("Query :: A %d %d\n", id, amount); // nurungg

    if ((depth = rb_find(tree, id, &node)) == -1) {
        puts("Not found!");

    } else {
        node->value.money += amount;
        
        set_level(&node->value);

        log.updown = UP;
        log.amount = amount;
        log_insert(node->value.log, log);

        printf("%d %d\n", depth, node->value.level);
    }
}

void treaverse_dfs_node(rb_node_t *node, int *id, int *money) {
    int i;

    if (node == NULL) return;

    treaverse_dfs_node(node->left, id, money);
    
    if (money[4] < node->value.money) {
        id[4]    = node->value.id;
        money[4] = node->value.money;
        
        for (int i = 4; i > 0; i--) {
            if (money[i-1] < money[i]) {
                int tmp_id, tmp_money;
                tmp_id    = id[i];
                tmp_money = money[i];

                id[i]    = id[i-1];
                money[i] = money[i-1];

                id[i-1]    = tmp_id;
                money[i-1] = tmp_money;

            } else {
                break;
            }
        }
    }

    treaverse_dfs_node(node->right, id, money);
}

void op_find_top_five(rb_tree_t *tree) {
    int id[5], money[5];
    int i;

    //printf("Query :: F\n"); // nurungg
    
    memset(id, 0, 5 * sizeof(int));
    memset(money, 0, 5 * sizeof(int));
    
    treaverse_dfs_node(tree->root, id, money);

    for (i = 0; i < 5; i++) {
        if (id[i] == 0) break;

        printf("%d %d\n", id[i], money[i]);
    }

    if (i == 0) {
        puts("Not found!");
    }
}

void op_print_log(rb_tree_t *tree) {
    int id, log_amount;
    rb_node_t *node;
    log_node_t *log;
    int i;

    scanf("%d %d", &id, &log_amount);
    //printf("Query :: R %d %d\n", id, log_amount); // nurungg

    if (rb_find(tree, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    log = node->value.log->head;
    for (i = 0; i < log_amount; i++) {
        if (log == NULL) break;

        printf("%d %d\n", log->value.updown, log->value.amount);
        
        log = log->next;
    }

    if (i == 0) {
        puts("0");
    }
}

void op_buy_area(rb_tree_t *tree) {
    int id, x, y, spent;
    rb_node_t *node, *origin;
    int approval = 0;

    log_t log_node, log_origin;

    scanf("%d %d %d %d", &id, &x, &y, &spent); 
    //printf("Query :: B %d %d %d %d\n", id, x, y, spent); // nurungg

    if (rb_find(tree, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    if (id != area_owner[x][y]) {
        if (spent >= area_price[x][y] && node->value.money >= spent) {
            approval = 1;
            
            node->value.money -= spent;
            set_level(&node->value);

            log_node.updown = DOWN;
            log_node.amount = spent;
            log_insert(node->value.log, log_node);

            if (area_owner[x][y] != -1) { // trade
                rb_find(tree, area_owner[x][y], &origin);

                origin->value.money += spent;
                set_level(&origin->value);

                log_origin.updown = UP;
                log_origin.amount = spent;
                log_insert(origin->value.log, log_origin);
            }

            area_price[x][y] = spent;
            area_owner[x][y] = id;
        }
    }

    printf("%d %d %d\n", approval, node->value.money, area_owner[x][y]);
}
