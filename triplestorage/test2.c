#include "tstorage.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	TStorage* st;
	int err;
	if (argc == 3) {
		err = tst_init(&st);
		if ( err ) {
			return -1;
		}
		err = tst_open(st, argv[1]);
		if ( err ) {
			return -2;
		}

		err = tst_save(st, argv[2]);
		if ( err ) {
			return -3;
		}
		tst_destroy(st);
		return 0;
	} else {
		printf("Usage: %s input output\n", argv[0]);
		return -1;
	}

}
