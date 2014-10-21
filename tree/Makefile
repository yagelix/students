all: tree_test

tree_test: tree.o tree_test.o
	gcc -o $@ $^

.c.o:
	gcc -c $< -o $@
