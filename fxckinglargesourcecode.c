#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RED		0
#define BLACK	1

/* Define structures */

struct log_s {
	int updown;
	int amount;
}

struct log_node_s {
	log_s value;
	log_node_s *next;
}

struct log_list_s {
	log_node_s *head;
}


// Information of each member
struct member_s {
	int	 id;
	char name[21];
	char phone[12];
	int	 x, y;
	int	 level;
	int	 money;
	log_list_s log;
};

// Red-Black Node type
struct rb_node_s {
	struct rb_node_s *parent;
	struct rb_node_s *left;
	struct rb_node_s *right;
	struct member_s	 value;
	int color;
};

// Red-Black Tree type
struct rb_tree_s {
	struct rb_node_s *root;
};

typedef struct member_s member_t;
typedef struct rb_node_s rb_node_t;
typedef struct rb_tree_s rb_tree_t;


/* Declare functions */
rb_tree_t	*rb_create();
rb_node_t	*rb_create_node();
void		rb_insert(rb_tree_t *tree, member_t value);
void		rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node);
void		rb_recoloring(rb_tree_t *tree, rb_node_t *node);
void		rb_restructuring(rb_tree_t *tree, rb_node_t *node);
int			rb_treaverse_node_dfs(rb_node_t *node, int depth);
void		rb_treaverse_dfs(rb_tree_t *tree);


/* Main function */
int main() {
	char filename[1024];
	FILE *input;
	
	rb_tree_t *tree;
	char op;

	// File in
	fputs("Input file name : ", stdout);
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
		}
		fclose(input);
	} // file input end 

	scanf("%c", &op);

	switch (op) {
	case 'I': // join member

	case 'P': // print member info

	case 'A': // add cash to account

	case 'F': // print 5 members who have most money

	case 'R': // 


	}

	return 0;
}

/* Function implementation */
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

void rb_insert(rb_tree_t *tree, member_t value) {
	rb_node_t *root;
	rb_node_t *vacant;
	rb_node_t *parent;

	if (tree->root == NULL) {
		root = rb_create_node();
		root->value = value;
		root->color = BLACK;
		
		tree->root = root;
		
	} else {
		vacant = tree->root;
		parent = NULL;
		
		while (vacant != NULL) {
			parent = vacant;

			if (value.id < vacant->value.id) { // go left
				vacant = vacant->left;

			} else { // go right 
				vacant = vacant->right;
			}
		}

		vacant = rb_create_node();
		vacant->parent = parent;
		vacant->value  = value;

		if (vacant->value.id < parent->value.id) {
			parent->left = vacant;
		} else {
			parent->right = vacant;
		}

		if (parent->color == RED) { // double red
			rb_remedy_double_red(tree, vacant);
		}
	}
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

void rb_remedy_double_red(rb_tree_t *tree, rb_node_t *node) {
	rb_node_t *parent;
	rb_node_t *uncle;

	// Double red situation guarantees the node has at least height of 3
	// So there is no need to doubt that the grand parent is NULL

	parent = node->parent;
	uncle  = get_sibling(parent);

	if (uncle->color == RED) { // recoloring
		rb_recoloring(tree, parent);
	} else { // restructuring
		rb_restructuring(tree, node);
	}
}
	
void rb_recoloring(rb_tree_t *tree, rb_node_t *node) {
	rb_node_t *parent;
	rb_node_t *sibling;

	parent  = node->parent;
	sibling = get_sibling(node);

	node->color    = BLACK;
	sibling->color = BLACK;
	
	if (parent != tree->root) {
		parent->color = RED;

		if (parent->parent->color == RED) { // propagation
			rb_remedy_double_red(tree, parent);
		}
	}

	// If parent is root vertex, there is no propagation
	// And root vertex still remain as BLACK node
}

void restructuring_setup(
	rb_node_t *node, rb_node_t *p, rb_node_t *l, rb_node_t *r, rb_node_t *lrc, rb_node_t *rlc) {

	rb_node_t *parent;
	rb_node_t *grand;

	parent = node->parent;
	grand  = parent->parent;
	
	if (grand->left == parent) {
		if (parent->left == node) { // left-left
			l = node;				/*     BLACK */
			r = grand;				/*     /     */
			p = parent;				/*   RED     */
			lrc = l->right;			/*   /       */
			rlc = p->right;			/* RED       */

		} else { // left-right
			l = parent;				/*   BLACK   */
			r = grand;				/*   /       */
			p = node;				/* RED       */
			lrc = p->left;			/*   \       */
			rlc = p->right;			/*   RED     */
		}

	} else {
		if (parent->left == node) { // right-left
			l = grand;				/*  BLACK    */
			r = parent;				/*      \    */
			p = node;				/*      RED  */
			lrc = p->left;			/*      /    */
			rlc = p->right;			/*     RED   */
			
		} else { // right-right
			l = grand;				/* BLACK     */
			r = node;				/*     \     */
			p = parent;				/*     RED   */
			lrc = p->left;			/*       \   */
			rlc = r->left;			/*       RED */
		}
	}
}

void rb_restructuring(rb_tree_t *tree, rb_node_t *node) {
	rb_node_t *parent;
	rb_node_t *left;
	rb_node_t *right;
	
	rb_node_t *left_right_child;
	rb_node_t *right_left_child;

	restructuring_setup(node, parent, left, right, left_right_child, right_left_child);
	
	// Change color
	parent->color = BLACK;
	left->color   = RED;
	right->color  = RED;

	// Renew children of parent
	parent->left  = left;
	parent->right = right;

	// Renew parent of each node
	parent->parent = node->parent->parent->parent;
	left->parent   = parent;
	right->parent  = parent;

	// Renew child of child
	left->right = left_right_child;
	right->left = right_left_child;

	// Renew parent of grand child
	left_right_child->parent = left;
	right_left_child->parent = right;

	// On restructuring, it doesn't propagate to upper layer
}

int rb_treaverse_node_dfs(rb_node_t *node, int depth) {

}

void rb_treaverse_dfs(rb_tree_t *tree) {

}
