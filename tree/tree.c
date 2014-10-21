#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

typedef struct _node {
	void* data;
	struct _node* left;
	struct _node* right;
} Node;

struct _tree {
	lessfunc lf;
	clone cl;
	destroy dt;
	Node* root;
};

/* tree initializer */
int tree_init(Tree** result, lessfunc lf, clone cl, destroy dt){
	Tree* r = (Tree*)malloc(sizeof(Tree));
	
	if (!r)
		return -1;
	
	r->lf = lf;
	r->cl = cl;
	r->dt = dt;
	r->root = NULL;
	*result = r;
	return 0;
}
/* tree inserter
 * inserts element into the tree
 * if replace set to true - substitues previous element with 
 * the same key value. Otherwise - appends new element.
 *
 * returns 0 in case of success, non-zero value otherwise.
 * */
int _node_insert(Tree* t, Node** r, void* element, int replace) {
	Node* n = *r;
	if ( ! n ) {
		n = *r = (Node*)malloc(sizeof(Node));
		if (! n ) return -2;
		n->data = t->cl(element);
		n->left = NULL;
		n->right = NULL;
		return 0;
	} 
	/* node exists */
	if ( t->lf(element, n->data) ) {

		return _node_insert(t, &n->left, element, replace);

	} else if ( t->lf(n->data, element) || !replace  ) {

		return _node_insert(t, &n->right, element, replace);

	} else { 
		t->dt(n->data);
		n->data = t->cl(element);
		return 0;
	}
}

int tree_insert(Tree* t, void* element, int replace) {
	return _node_insert(t, &t->root, element, replace);
}

/* Finds given element in the tree 
 * In case of success, makes result pointed to the found element and returns1
 * Otherwise, returns 0.
 */


int _node_walk(Tree* t, Node* n, void* element, void** result) {
	if ( ! n ) return 0;
	if ( t->lf(n->data, element) ){
		return _node_walk(t, n->right, element, result);
	} else if ( t->lf(element, n->data) ) {
		return _node_walk(t, n->left, element, result);
	} else {
		*result = n->data;
		return 1;
	}
}

int tree_find(Tree* t, void* element, void** result) {
	return _node_walk(t, t->root, element, result);
}

int _node_find_all(Tree* t, Node* n, 
		void* from, void* to, iterator it, void* it_data) {
	int k = 0;
	if ( !n ) 
		return 0;

	if(!from || !t->lf(n->data, from) ) {
		if (t->lf(from, n->data)) {
			fprintf(stderr, "Walking left\n");
			k += _node_find_all(t, n->left, from, to,  it, it_data);
		}
		if ( (!to || !t->lf(to, n->data)) ) {
			if (!it(n->data, it_data)) 
				return k;
			k += 1;
		}
	}
	if (!to || !t->lf(to, n->data)){

		fprintf(stderr, "Walking right\n");
		k += _node_find_all(t, n->right, from, to, it, it_data);
	}
	return k;
}

/* Finds all elements between from and to
 * from or to may be NULL  - in this case the search is one-side unbounded.
 *
 * calls iterator for each element found.
 * If elements exists, returns 1, otherwise - 0.
 */

int tree_find_all(Tree* t, void* from, void* to, iterator it, void* it_data) {
	return _node_find_all(t, t->root, from, to, it, it_data);

}


/* Deletes given element
 *
 */

/* Destroys entire tree
 *
 */



