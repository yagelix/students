#include "tree.h"
#include "stdio.h"

void* int_clone(void* data) {
	return data;
}

int int_less(void* a, void* b) {
	return (int)a < (int)b;
}

void int_destroy(void* a) {
}

int print_int(void* a, void* data) {
	printf("Num found: %d\n", ((int)a >> 1));
	return 1;
}

int main() {
	int values[] = {5,4,1,5,6,7,9,10,8,8,8,8,8,9,5};
	Tree* t;
	int i;
	tree_init(&t, int_less, int_clone, int_destroy);
	for( i = 0; i < sizeof(values)/sizeof(int); i++ ){
		tree_insert(t, (void*)(values[i]<<1 | 1), 0);
	}
	tree_find_all(t, NULL, (void*)(5<<1|1), print_int, NULL);
	return 0;
}
