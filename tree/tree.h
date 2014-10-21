#ifndef __TREE_H__
#define __TREE_H__


/* base struct */
typedef struct _tree Tree;


/* comparator interface */
typedef int (*lessfunc)(void* a, void* b);
/* cloner interface */
typedef void* (*clone)(void* data);
/* terminator interface */
typedef void (*destroy)(void* data);
/* tree walker interface */
typedef int (*iterator)(void* result, void* data);

/* tree initializer */
int tree_init(Tree** result, lessfunc lf, clone cl, destroy dt);
/* tree inserter
 * inserts element into the tree
 * if replace set to true - substitues previous element with 
 * the same key value. Otherwise - appends new element.
 *
 * returns 0 in case of success, non-zero value otherwise.
 * */
int tree_insert(Tree* t, void* element, int replace);

/* Finds given element in the tree 
 * In case of success, makes result pointed to the found element and returns1
 * Otherwise, returns 0.
 */
int tree_find(Tree* t, void* element, void** result);

/* Finds all elements between from and to
 * from or to may be NULL  - in this case the search is one-side unbounded.
 *
 * calls iterator for each element found.
 * If elements exists, returns 1, otherwise - 0.
 */
int tree_find_all(Tree* t, void* from, void* to, iterator it, void* it_data);


/* Deletes given element
 *
 */
int tree_delete(Tree* t, void* element);

/* Destroys entire tree
 *
 */
int tree_destroy(Tree* t);

/* Helper functions */
int int_less(void* a, void* b);
int str_less(void* a, void* b);
void* int_clone(void* a);
void* str_clone(void* a);
void int_destroy(void* a);
void str_destroy(void* a);

#endif /* __TREE_H__ */
