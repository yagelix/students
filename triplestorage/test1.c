#include "tstorage.h"
#include <stdio.h>
#include <stdarg.h>

static void __log(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}


int main() {
	int err;
	TStorage* st;
	Statement stmt;
	
	__log("Initializing storage...");
	if ( (err = tst_init(&st)) ) {
		__log("Error while initializing storage %d", err);
		return -1;
	} 
	
	
	__log("Creating test statements");
	
	stmt_init(&stmt, L"A", L"Mama", L"myla", L"ramu");
	
	if ((err = tst_set(st,  stmt))) {
		__log ( "Error while setting %d", err);
	}

	stmt_init(&stmt, L"A", L"rama", L"byla", L"Very white");
	if ((err = tst_set(st, stmt))) {
		__log ( "Error while setting %d", err);
	}


	__log("Trying to save");
	if ((err=tst_save(st, "test1.db"))) {
		__log ("Error while saving %d", err);
	}
	
	__log("Destroying storage...");
	tst_destroy(st);
	return 0;

}
