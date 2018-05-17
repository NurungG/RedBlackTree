/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../rbt.h"


/* Defines */
#define EXIT_STATUS     0
#define CONTINUE        1

#define UP              1
#define DOWN            0

#define RANK_MAX        100


/* Structures */

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

typedef struct log_node_s log_node_t;
typedef struct log_list_s log_list_t;
typedef struct member_s member_t;


/* Declare function prototype */

// Log(list) implementation
log_list_t *log_create();
log_node_t *log_create_node();
void        log_insert(log_list_t *list, int ud, int amt);

// Member creation
member_t   *member_create();

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
void Init() {
    // Initiate global variables
    int i;

    all_members = rb_create();
    memset(area_owner, -1, 1001 * 1001 * sizeof(int));

    zero_node = rb_create_node();
    zero_node->value = (void*)member_create();
}

int Setup() {
    // Setup existing members from member-list file
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

    while (1) {
        member = member_create();
        if ( fscanf(input, "%u %s %s %d %d %d %d",
                &id, member->name, member->phone,
                &member->x, &member->y, &member->level, &member->money) < 0 ) {
            break;
        }
        
        rb_insert(all_members, id, (void*)member);
        area_owner[member->x][member->y] = id;
    }
    free(member);
    fclose(input);

    traverse_dfs(all_members); // set rank array

    return 0;
}

void Execute() {
    // Execute queries

    char op;
    while (1) {
        scanf("%c", &op);
        if (execute_operation(op) == EXIT_STATUS) {
            break;
        }
    }
}

int execute_operation(char op) {
    // Execute an operation (by translating a query)
    
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

void op_join_member() {
    // Join to this game

    rb_key_t id;
    member_t *member;
    int approval, depth;

    member = member_create();
    
    scanf("%u %s %s %d %d",
        &id, member->name, member->phone, &member->x, &member->y);

    if ((approval = rb_insert(all_members, id, (void*)member)) == 0) {
        // If there is no owner in starting area, it becomes belonging of him(or her)
        if (area_owner[member->x][member->y] == -1) {
            area_owner[member->x][member->y] = id;
        }
    } else {
        free(member);
    }
    
    depth = rb_find(all_members, id, NULL);

    printf("%d %d\n", depth, approval+1);
}

void op_print_info() {
    // Print information of the member
    
    rb_key_t    id;
    int         depth;
    rb_node_t  *node;
    member_t   *info;

    scanf("%u", &id);

    if ((depth = rb_find(all_members, id, &node)) == -1) {
        puts("Not found!");

    } else {            
        info = (member_t*)node->value;
        printf("%s %s %d %d %u\n",
            info->name, info->phone, info->level, info->money, depth);
    }
}

void set_level(member_t *member) {
    // Set the level depending on current money

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

int rank_cmp(rb_node_t *a, rb_node_t *b) {
    int money_a, money_b;

    money_a = ((member_t*)a->value)->money;
    money_b = ((member_t*)b->value)->money;

    if (money_a > money_b) {
        return 1;
    } else if (money_a == money_b) {
        if (a->key < b->key) {
            return 1;
        }
    }
    return 0;
}

void reset_bound() {
    rb_node_t *node = rb_create_node();
    node->value = (void*)member_create();

    ((member_t*)node->value)->money = bound_money;
    node->key                       = bound_id;

    if (rank_cmp(rank[RANK_MAX-1], node)) { // increase bound
        bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
        bound_id    = rank[RANK_MAX-1]->key;
    } else if (rank_cmp(node, rank[4])) { // renew rank array
        traverse_dfs(all_members);

        bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
        bound_id    = rank[RANK_MAX-1]->key;
    }
    
    free((member_t*)node->value);
    free(node);

    b++;
}

void op_add_cash() {
    // Add cash to the account
    rb_key_t    id;
    int         amount, depth;
    rb_node_t  *node;
    member_t   *info;

    int i;

    scanf("%d %d", &id, &amount);

    if ((depth = rb_find(all_members, id, &node)) == -1) {
        puts("Not found!");

    } else {
        info = (member_t*)node->value;
        info->money += amount;
        set_level(info);

        log_insert(info->log, UP, amount);

        printf("%d %d\n", depth, info->level);

        // renew rank
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


void traverse_dfs(rb_tree_t *tree) {
    for (int i = 0; i < RANK_MAX; i++) {
        rank[i] = zero_node;
    }
    traverse_dfs_node(tree->root);
    bound_id = rank[RANK_MAX-1]->key;
    bound_money = ((member_t*)rank[RANK_MAX-1]->value)->money;
    a++;
}

void traverse_dfs_node(rb_node_t *node) {
    member_t *info;
    int i;

    if (node == NULL) return;

    traverse_dfs_node(node->left);

    info = (member_t*)node->value;
    if (info->money > ((member_t*)rank[RANK_MAX-1]->value)->money) {
        ((member_t*)rank[RANK_MAX-1]->value)->is_ranked = 0;
        for (i = RANK_MAX-1; i > 0; i--) {
            if (((member_t*)rank[i-1]->value)->money < info->money) {
                rank[i] = rank[i-1];
            } else {
                break;
            }
        }
        rank[i] = node;
        info->is_ranked = 1;
    }

    traverse_dfs_node(node->right);
}

void op_find_top_five() {
    int i;

    for (i = 0; i < 5; i++) {
        if (rank[i] == zero_node) break;

        printf("%d %d\n", rank[i]->key, ((member_t*)rank[i]->value)->money);
    }

    if (i == 0) {
        puts("Not found!");
    }
}

void op_print_log() {
    // Print log of the account

    rb_key_t id;
    int print_size;
    rb_node_t *node;
    log_node_t *log;
    int i;

    scanf("%u %d", &id, &print_size);

    if (rb_find(all_members, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    log = ((member_t*)node->value)->log->head;
    for (i = 0; i < print_size; i++) {
        if (log == NULL) break;

        printf("%d %d\n", log->updown, log->amount);        
        log = log->next;
    }

    if (i == 0) {
        puts("0");
    }
}

void op_buy_area() {
    // Buy the area at the price(or more)

    rb_key_t id;
    int x, y, spent;
    rb_node_t *node, *origin;
    member_t *info;
    int approval;

    int i;

    scanf("%u %d %d %d", &id, &x, &y, &spent); 

    if (rb_find(all_members, id, &node) == -1) {
        puts("Not found!");
        return;
    }

    approval = 0;
    info = (member_t*)node->value;

    if (id != area_owner[x][y]) { // purchase is affordable only when it's area of others

        // The member should pay affordable price
        // and current account(money) of the member should be enough to pay over
        if (spent >= area_price[x][y] && info->money >= spent) {
            approval = 1;

            if (area_owner[x][y] != -1) { // case of trade
                rb_find(all_members, area_owner[x][y], &origin);
                
                info = (member_t*)origin->value;

                info->money += spent;
                set_level(info);
                log_insert(info->log, UP, spent);

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

            info = (member_t*)node->value;

            info->money -= spent;
            set_level(info);
            log_insert(info->log, DOWN, spent);

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

            // renew area info
            area_price[x][y] = spent;
            area_owner[x][y] = id;
        }
    }

    printf("%d %d %d\n", approval, info->money, area_owner[x][y]);
}


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

    node->next   = NULL;
    node->updown = 0;
    node->amount = 0;

    return node;
}

void log_insert(log_list_t *list, int ud, int amt) {
    log_node_t *node;

    node = log_create_node();
    node->updown = ud;
    node->amount = amt;

    node->next = list->head;
    list->head = node;
}

member_t *member_create() {
    member_t *member;
    
    if ((member = malloc(sizeof(member_t))) == NULL) {
        return NULL;
    }

    memset(member, 0, sizeof(member_t));
    member->log = log_create();

    return member;
}
