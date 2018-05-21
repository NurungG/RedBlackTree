/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* Defines */

#define RED             0
#define BLACK           1

#define EXIT_STATUS     0
#define CONTINUE        1

#define UP              1
#define DOWN            0

#define RANK_MAX        10

typedef unsigned int rb_key_t;


/* Structures */

// Red-Black Node structure
struct rb_node_s {
    struct rb_node_s *parent;
    struct rb_node_s *left;
    struct rb_node_s *right;

    rb_key_t key;
    void    *value;
    int      color; // 0(RED) or 1(BLACK)
};

// Red-Black Tree structure
struct rb_tree_s {
    struct rb_node_s *root;
};


// Log node structure (including information about each log of account)
struct log_node_s {
    int updown;
    int amount;
    struct log_node_s *next;
};

// Log list structure
struct log_list_s {
    struct log_node_s *head;
};

// Information of each member
struct member_s {
    char name[21];
    char phone[12];
    int  x, y;
    int  level;
    int  money;
    struct log_list_s *log;
    int  is_ranked;
};

typedef struct rb_node_s rb_node_t;
typedef struct rb_tree_s rb_tree_t;

typedef struct log_node_s log_node_t;
typedef struct log_list_s log_list_t;
typedef struct member_s member_t;


/* Declare function prototype */

// Red-Black Tree implementation
rb_tree_t  *rb_create();
rb_node_t  *rb_create_node();
int         rb_insert(rb_tree_t *tree, rb_key_t ikey, void *value);
void        rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node);
int         rb_find(rb_tree_t *tree, rb_key_t skey, rb_node_t **node);

// Log(list) implementation
log_list_t *log_create();
log_node_t *log_create_node();
void        log_delete();
void        log_insert(log_list_t *list, int ud, int amt);

// Member creation
member_t   *create_member();
void        delete_member();

// Main wrapper functions
void        Init();
int         Setup();
void        Execute();

// Execute wrapper functions
int         execute_operation(char op);
void        op_join_member();
void        op_print_info();
void        op_add_cash();
void        op_find_top_five();
void        op_print_log();
void        op_buy_area();

// Rank setup by depth first search traversal
void        traverse_dfs(rb_tree_t *tree);
void        traverse_dfs_node(rb_node_t *node);


/* Global variables */
rb_tree_t      *all_members;
int             area_price[1001][1001];
int             area_owner[1001][1001];

rb_node_t      *rank[RANK_MAX];
rb_node_t      *zero_node;
int             bound_id, bound_money;

int a, b, c, d, e, f, g;


/* Main function */
int main() {
    time_t stime = 0, etime = 0;
    float gap;

    stime = clock();

    Init();
    Setup();
    Execute();

    printf("traverse calls                   :: %d\n", a);
    printf("reset_bound calls                :: %d\n", b);
    printf("ranked member buy                :: %d\n", c);
    printf("ranked member add cash           :: %d\n", d);
    printf("ranked member sell               :: %d\n", e);
    printf("new member ranked by sell        :: %d\n", f);
    printf("new member ranked in by add cash :: %d\n", g);

    etime = clock();

    gap = (float)(etime-stime)/(CLOCKS_PER_SEC);
    
    printf("execution time : %f sec\n", gap);
    return 0;
}


/* Function implementation */

/* Initiate global variables */
void Init() {
    all_members = rb_create();
    memset(area_owner, -1, 1001 * 1001 * sizeof(int));

    zero_node = rb_create_node();
    zero_node->value = (void*)create_member();
}

/* Setup existing members from member-list file */
int Setup() {
    char filename[1024];
    FILE *input;

    member_t *member;
    rb_key_t id;

    fputs("Input filename : ", stdout);
    fgets(filename, 1024, stdin);

    filename[strlen(filename)-1] = 0;

    if ((input = fopen(filename, "r")) == NULL) {
        fputs("fopen() error\n", stderr);
        exit(1);
    }

    // Load data from file
    while (1) {
        member = create_member();

        // Read data
        if ( fscanf(input, "%u %s %s %d %d %d %d",
                &id, member->name, member->phone,
                &member->x, &member->y, &member->level, &member->money) < 0 ) {
            break;
        }
        
        // Insert to the Red-Black Tree
        rb_insert(all_members, id, (void*)member);

        // Set owner information
        area_owner[member->x][member->y] = id;
    }
    
    delete_member(member);
    fclose(input);

    // Set rank array
    traverse_dfs(all_members);

    return 0;
}

/* Execute queries */
void Execute() {
    char op;
    while (1) {
        scanf("%c", &op);
        if (execute_operation(op) == EXIT_STATUS) {
            break;
        }
    }
}

/* Execute an operation (by translating a query) */
int execute_operation(char op) {    
    switch (op) {

    case 'I' : // join
        op_join_member();
        break;
    case 'P' : // info
        op_print_info();
        break;
    case 'A' : // add
        op_add_cash();
        break;
    case 'F' : // rank
        op_find_top_five();
        break;
    case 'R' : // log
        op_print_log();
        break;
    case 'B' : // buy
        op_buy_area();
        break;
    case 'Q' : // exit
        return EXIT_STATUS;
    default :
        printf("Invalid operation %c\n", op);
    }

    getc(stdin); // to flush '\n' character

    return CONTINUE;
}

/* Join to this game */
void op_join_member() {
    rb_key_t id;
    member_t *member;
    int approval, depth;

    member = create_member();
    
    scanf("%u %s %s %d %d",
        &id, member->name, member->phone, &member->x, &member->y);

    // Insert
    if ((approval = rb_insert(all_members, id, (void*)member)) == 0) {
        // If there is no owner in starting area, it becomes belonging of him(or her)
        if (area_owner[member->x][member->y] == -1) {
            area_owner[member->x][member->y] = id;
        }
    } else {
        delete_member(member);
    }
    
    // Get depth of the node
    depth = rb_find(all_members, id, NULL);

    printf("%d %d\n", depth, approval+1);
}

/* Print information of the member */
void op_print_info() {
    rb_key_t    id;
    int         depth;
    rb_node_t  *node;
    member_t   *info;

    scanf("%u", &id);

    // Find
    if ((depth = rb_find(all_members, id, &node)) == -1) {
        puts("Not found!");

    } else {
        info = (member_t*)node->value;
        printf("%s %s %d %d %u\n",
            info->name, info->phone, info->level, info->money, depth);
    }
}

/* Set the level depending on current money */
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

/* Do comparison node a with node b (compare money mainly, and then id) */
int rank_cmp(rb_node_t *a, rb_node_t *b) {
    int money_a, money_b;

    money_a = ((member_t*)a->value)->money;
    money_b = ((member_t*)b->value)->money;

    // If a has money more than b, simply return 1
    if (money_a > money_b) {
        return 1;

    // Else, check the id
    } else if (money_a == money_b) {

        // If id of a is smaller than of b, return 1
        if (a->key < b->key) {
            return 1;
        }
    }

    // 0 means that node b is preceding in order
    return 0;
}

/* If account of ranked member is changed, reset the boundary of money and id */
void reset_bound() {
    rb_node_t *node = rb_create_node();
    node->value = (void*)create_member();

    ((member_t*)node->value)->money = bound_money;
    node->key                       = bound_id;

    // If boundary should be increased
    if (rank_cmp(rank[RANK_MAX-1], node)) {
        bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
        bound_id    = rank[RANK_MAX-1]->key;

    // Else if top 5 ranked member has been under the boundary
    } else if (rank_cmp(node, rank[4])) {
        // Renew the rank array -> O(n) time
        traverse_dfs(all_members);

        bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
        bound_id    = rank[RANK_MAX-1]->key;
    }
    
    delete_member((member_t*)node->value);
    free(node);

    b++;
}

/* Add cash to the account */
void op_add_cash() {
    rb_key_t    id;
    int         amount, depth;
    rb_node_t  *node;
    member_t   *info;

    int i;

    scanf("%u %d", &id, &amount);

    // If there is no node corresponding to given id
    if ((depth = rb_find(all_members, id, &node)) == -1) {
        puts("Not found!");

    // Else, add cash
    } else {
        info = (member_t*)node->value;

        // Add cash
        info->money += amount;

        // Reset level of the member
        set_level(info);

        // Append log
        log_insert(info->log, UP, amount);

        printf("%d %d\n", depth, info->level);

        // Renew rank
        if (info->is_ranked == 1) {
            for(i = 0; i < RANK_MAX; i++) {
                if (rank[i] == node) break;
            }
            for (; i > 0; i--) {
                if (rank_cmp(node, rank[i-1])) {
                    rank[i] = rank[i-1];
                } else {
                    break;
                }
            }
            rank[i] = node;

            d++;

        } else {
            if (rank_cmp(node, rank[RANK_MAX-1])) {
                ((member_t*)rank[RANK_MAX-1]->value)->is_ranked = 0;
                info->is_ranked = 1;

                for (i = RANK_MAX-1; i > 0; i--) {
                    if (rank_cmp(node, rank[i-1])) {
                        rank[i] = rank[i-1];
                    } else {
                        break;
                    }
                }
                rank[i] = node;
                reset_bound();

                g++;
            }
        }
    }
}

/* Find ranked members by depth first traversal */
void traverse_dfs(rb_tree_t *tree) {
    // Flush the rank array
    for (int i = 0; i < RANK_MAX; i++) {
        rank[i] = zero_node;
    }

    // Do traversal
    traverse_dfs_node(tree->root);

    // Set boundary
    bound_id = rank[RANK_MAX-1]->key;
    bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
    
    a++;
}

/* Do a depth first traversal of node, maintaining rank array */
void traverse_dfs_node(rb_node_t *node) {
    member_t *info;
    int i;

    if (node == NULL) return;

    // Do the left node first (smallest id gets in first)
    traverse_dfs_node(node->left);

    // Compare with last of ranked member
    info = (member_t*)node->value;
    if (info->money > ((member_t*)rank[RANK_MAX-1]->value)->money) {
        // Swap out
        ((member_t*)rank[RANK_MAX-1]->value)->is_ranked = 0;

        // Find vacant
        for (i = RANK_MAX-1; i > 0; i--) {
            if (((member_t*)rank[i-1]->value)->money < info->money) {
                rank[i] = rank[i-1];
            } else {
                break;
            }
        }

        // Swap in
        rank[i] = node;
        info->is_ranked = 1;
    }

    // Do the right node last, (largest id treated last)
    traverse_dfs_node(node->right);
}

/* Print top 5 members */
void op_find_top_five() {
    int i;

    // Print ranked member, directly access to ranked array
    // Time complexity :: O(1)
    for (i = 0; i < 5; i++) {
        if (rank[i] == zero_node) break;

        printf("%d %d\n", rank[i]->key, ((member_t*)rank[i]->value)->money);
    }

    if (i == 0) {
        puts("Not found!");
    }
}

/* Print log of the account */
void op_print_log() {
    rb_key_t    id;
    int         print_size;
    rb_node_t  *node;
    log_node_t *log;
    
    int i;

    scanf("%u %d", &id, &print_size);

    // If there is no node corresponding to given id
    if (rb_find(all_members, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    // Print logs
    log = ((member_t*)node->value)->log->head;
    for (i = 0; i < print_size; i++) {
        if (log == NULL) break;

        printf("%d %d\n", log->updown, log->amount);        
        log = log->next;
    }

    // Case of no log
    if (i == 0) {
        puts("0");
    }
}

/* Buy the area at the price(or more) */
void op_buy_area() {
    rb_key_t    id;
    int         x, y, spent;

    rb_node_t  *node, *origin;
    member_t   *info;
    int         approval;

    int i;

    scanf("%u %d %d %d", &id, &x, &y, &spent); 

    // If there is no node corresponding to the given id
    if (rb_find(all_members, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    approval = 0;
    info = (member_t*)node->value;

    // Do purchase only when it's area of others
    if (id != area_owner[x][y]) {

        // The member should pay affordable price
        // and current account(money) of the member should be enough to pay it over
        if (spent >= area_price[x][y] && info->money >= spent) {
            // Set approval flag to 1
            approval = 1;

            // Case of trade
            if (area_owner[x][y] != -1) {
                // Find the owner of the area
                rb_find(all_members, area_owner[x][y], &origin);
                
                info = (member_t*)origin->value;

                // Add cash
                info->money += spent;
                set_level(info);
                log_insert(info->log, UP, spent);

                // Renew the ranked array
                if (info->is_ranked == 1) {
                    for (i = 0; i < RANK_MAX; i++) {
                        if (rank[i] == origin) break;
                    }
                    for (; i > 0; i--) {
                        if (rank_cmp(origin, rank[i-1])) {
                            rank[i] = rank[i-1];
                        } else {
                            break;
                        }
                    }
                    rank[i] = origin;
                    e++;

                } else {
                    if (rank_cmp(origin, rank[RANK_MAX-1])) {
                        ((member_t*)rank[RANK_MAX-1]->value)->is_ranked = 0;
                        info->is_ranked = 1;

                        for (i = RANK_MAX-1; i > 0; i--) {
                            if (rank_cmp(origin, rank[i-1])) {
                                rank[i] = rank[i-1];
                            } else {
                                break;
                            }
                        }
                        rank[i] = origin;
                        reset_bound();

                        f++;
                    }
                }
            }

            // Proceed the purchase
            info = (member_t*)node->value;

            // Decrease the money of account
            info->money -= spent;
            set_level(info);
            log_insert(info->log, DOWN, spent);

            // Renew the ranked array
            // There is no need to doubt to getting in a new node to ranked array
            if (info->is_ranked == 1) {
                for (i = 0; i < RANK_MAX; i++) {
                    if (rank[i] == node) break;
                }

                for (; i < RANK_MAX-1; i++) {
                    if (rank_cmp(rank[i+1], node)) {
                        rank[i] = rank[i+1];
                    } else {
                        break;
                    }
                }
                rank[i] = node;

                reset_bound();

                c++;
            }

            // Renew area info
            area_price[x][y] = spent;
            area_owner[x][y] = id;
        }
    }

    printf("%d %d %d\n", approval, info->money, area_owner[x][y]);
}

/* Create log list */
log_list_t *log_create() {
    log_list_t *list = NULL;
    
    if ((list = malloc(sizeof(log_list_t))) == NULL) {
        return NULL;
    }

    list->head = NULL;

    return list;
}

/* Create log node */
log_node_t *log_create_node() {
    log_node_t *node = NULL;

    if ((node = malloc(sizeof(log_node_t))) == NULL) {
        return NULL;
    }

    node->next   = NULL;
    node->updown = 0;
    node->amount = 0;

    return node;
}

/* Delete log list */
void log_delete(log_list_t *list) {
    log_node_t *log, *tmp;

    log = list->head;
    while (log != NULL) {
        tmp = log->next;
        free(log);
        log = tmp;
    }

    free(list);
}

/* Insert log to list */
void log_insert(log_list_t *list, int ud, int amt) {
    log_node_t *node;

    node = log_create_node();
    node->updown = ud;
    node->amount = amt;

    node->next = list->head;
    list->head = node;
}

/* Create member */
member_t *create_member() {
    member_t *member;
    
    if ((member = malloc(sizeof(member_t))) == NULL) {
        return NULL;
    }

    memset(member, 0, sizeof(member_t));
    member->log = log_create();

    return member;
}

/* Delete member */
void delete_member(member_t *member) {
    log_delete(member->log);
    free(member);
}

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
