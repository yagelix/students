#include "tstorage.h"
#include <stdio.h>


int _stmt_printout(Statement stmt, void* data) {
	printf("<%ls, %ls, %ls, %ls>\n", stmt[0], stmt[1], stmt[2], stmt[3]);
	return 1;
}

int _stmt_counter(Statement stmt, void* data) {
	int* cnt = (int*)data;
	(*cnt)++;
	return 1;
}

int main(int argc, char* argv[]) {
	TStorage* st;
	int err;
	if (argc == 3) {
		int count;
		Statement mask;
		err = tst_init(&st);
		if ( err ) {
			return -1;
		}
		err = tst_open(st, argv[1]);
		if ( err ) {
			return -2;
		}
		stmt_init(&mask, L"A", NULL, NULL, NULL);
		err = tst_get(st, mask, _stmt_printout, NULL);

		if ( err ) return -4;

		count = 0;
		stmt_init(&mask, L"A", NULL, NULL, NULL);
		err = tst_get(st, mask, _stmt_counter, (void*)&count);
		if ( err )  return -5;

		printf("A statements: %d\n", count);
		
	

		err = tst_save(st, argv[2]);
		if ( err )  return -3;
		
		// remove statements
		//
		err = tst_unset(st, mask);
		if ( err ) return -6;
		count = 0;
		err = tst_get(st, mask, _stmt_counter, (void*)&count);
		if ( err )  return -7;
		printf("A statements after deletion: %d\n", count);

		tst_get(st, mask, _stmt_printout, NULL);

		tst_destroy(st);
		return 0;
	} else {
		printf("Usage: %s input output\n", argv[0]);
		return -1;
	}

}
