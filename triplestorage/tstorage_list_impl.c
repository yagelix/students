#include "tstorage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>

void logerr(const char* err, ...) {
	va_list ap;
	va_start(ap, err);
	vfprintf(stderr, err, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

typedef struct _stmt {
	wchar_t* term[STATEMENT_SIZE];
	struct _stmt* next;
} _Statement;

struct _tstorage {
	_Statement* statements;
};


void stmt_init(Statement* stmt, ...) {
	va_list ap;
	int k = 0;
	va_start(ap, stmt);
	for(k =0; k < STATEMENT_SIZE; k++) {
		(*stmt)[k] = va_arg(ap, wchar_t *);
	}
	va_end(ap);
}

int stmt_match(_Statement* stmt, Statement mask) {
	int k;
	for (k = 0; k < STATEMENT_SIZE; k++ ){
		if ( mask[k] && wcscmp(stmt->term[k], mask[k])) 
			return 0;
	}
	return 1;				
}		

int stmt_extract(_Statement* stmt, Statement* result) {
	int k;
	for ( k = 0; k < STATEMENT_SIZE; k++ ) {
		(*result)[k] = stmt->term[k];
	}
	return 0;
}


int tst_init(TStorage** ts) {
	*ts = (TStorage*)malloc(sizeof(TStorage));
	if (! *ts) 
		return -1;
	(*ts)->statements = NULL;
	return 0;
}

void _stmt_destroy(_Statement* stmt, int destroy_next) {
	int k;
	_Statement* next = stmt->next;
	for ( k = 0; k < STATEMENT_SIZE; k++ ){
		free(stmt->term[k]);
	}
	free(stmt);
	if ( next  && destroy_next) 
		_stmt_destroy(next, 1);
}

int _read_word(wchar_t* buf, FILE* f) {
	long pos = ftell(f);
	while (!feof(f)) {
		wint_t c = fgetwc(f);
		//logerr("Читаем sym '%x'", c);
		if ( iswalnum(c) ) {
			//logerr("Is alnum");
			*buf = c;
			buf++;
		} else if (iswspace(c)) {
			*buf = 0;
			return 0;
		} else {
			break;
		}
	} 
	if(feof(f)) {
		*buf = 0;
		return 0;
	}

	fseek(f, pos, SEEK_SET);
	return -5;
}

int _read_quote(FILE* f) {
	long pos = ftell(f);
	wint_t x;
	if ( feof(f) ){
		return -1;
	}
	if ( ( x = fgetwc(f)) == L'\"') {
		return 0;
	}
	logerr("Cannot read quote: '%lc'", x);
	fseek(f, pos, SEEK_SET);
	return -2;

}

int _read_nonescaped(wchar_t** buf, FILE* f) {
	wint_t c = fgetwc(f);
	if ( c == L'\\' ) {
		ungetwc(c, f);
		return  -1;
	} else {
		**buf = c;
		(*buf)++;
	}
	return 0;
}

int _read_escaped(wchar_t** buf, FILE* f) {
	wint_t c = fgetwc(f);
	if ( feof(f) ) {
		return -2;
	}
	if( c != L'\\') {
		ungetwc(c, f);
		return -1;
	}
	
	c = fgetwc(f);
	if ( feof(f)) {
		return -3;
	}

	switch (c) {
	case L't': 
		**buf = L'\t';
		break;
	case L'n': 
		**buf = L'\n';
		break;
	case L'r':
		**buf = L'\r';
		break;
	default:
		**buf = c;
	}
	(*buf)++;
	return 0;
}

int _read_until_quote(wchar_t* buf, FILE* f) {
	
	wint_t c;
	long pos = ftell(f);
	c = fgetwc(f);
	if (c == L'\"') 
		return 0;
	ungetwc(c, f);
	if ( _read_nonescaped(&buf, f) ) {
		if ( _read_escaped(&buf, f) ) {
			fseek(f, pos, SEEK_SET);
			return -9;
		}
	}
	return _read_until_quote(buf, f);
	
}

int _read_qstring(wchar_t* buf, FILE* f) {
	long pos = ftell(f);
	if (_read_quote(f)) 
		return -6;
	if (_read_until_quote(buf, f)) {
		fseek(f, pos, SEEK_SET);
		return -7;
	}
	return 0;

}


int _read_term(wchar_t* buf, FILE* f) {
	long pos = ftell(f);
	if (_read_word(buf, f)) {
		logerr("Unable to read word at %ld, trying qstring", pos);
		if (_read_qstring(buf, f)) {
			fseek(f, pos, SEEK_SET);
			return -4;
		}
	}
	return 0;
}

int _read_statement(_Statement** st, FILE* f) {
	int k;
	wchar_t buf[4096];
	*st = (_Statement*)malloc(sizeof(_Statement));
	memset(*st, 0, sizeof(_Statement));

	for (k = 0; k < STATEMENT_SIZE; k++ ){
		if ( _read_term(buf, f) ) {
			logerr("Cannot read term at pos %ld", ftell(f));
			_stmt_destroy(*st, 1);
			return -1;
		}
		(*st)->term[k] = wcsdup(buf);
	}
	return 0;
}

int tst_open(TStorage* ts, const char* filename) {
	FILE* f;
	_Statement** current = &ts->statements;
	char *locale = setlocale(LC_CTYPE, "ru_RU.UTF-8");
	f = fopen(filename, "rb");

	if ( !f ) {
		logerr("Cannot open file");
		return -1;
	}
	fwide(f, 0);		
	while(!feof(f)) {
		if (_read_statement(current, f)) {
			logerr("Cannot read statement at pos %ld", ftell(f));
			return -2;
		}
		current = &(*current)->next;
	}
	return 0;
}

int _write_term_escaped(const wchar_t* term, FILE* f) {
	const wchar_t *k;
	fwprintf(f, L"\"");
	for(k = term; *k; k++ ) {
		if ( *k == '\\' || *k == '\"' ) {
			fwprintf(f, L"\\%lc", *k);	
		} else if (*k == '\n' ) {
			fwprintf(f, L"\\n");
		} else if (*k == '\r') {
			fwprintf(f, L"\\r");
		} else {
			fwprintf(f, L"%lc", *k);
		}
	}
	fwprintf(f, L"\"");
	return 0;
		
}

int _write_term(const wchar_t* term, FILE* f) {
	const wchar_t* k;
	for (k = term; *k; k++ ) {
	     if (iswspace(*k) || *k == '\\' || *k=='\"' || *k == '\'') {

		return _write_term_escaped(term, f);       		       
	     }
     	}
	if (fwprintf(f, term) != wcslen(term)) {
		logerr("Cannot write term '%s'", term);
		return -5;
	}
	return 0;

}

int _write_statement(_Statement* stmt, FILE* f) {
	if (stmt) {
		int i;
		for(i = 0; i < STATEMENT_SIZE; i++ ) {
			if (i != 0 ) {
				if ( fwprintf(f, L" ") != 1)  {
					logerr("Cannot write whitespace");
					return -5;
				}
			}

			if ( _write_term(stmt->term[i], f)){
				logerr("Cannot write term %d", i);
				return -1;
			}
		}
		if ( fwprintf(f, L"\n") != 1) {
			logerr("Cannot finish statement");
			return -6;
		}
		return _write_statement(stmt->next, f);
	} else {
		return 0;
	}	

}

int tst_save(TStorage* ts, const char* filename) {
	FILE* f;
	int err = 0;
	_Statement* current = ts->statements;
	f = fopen(filename, "w+");
	if ( !f ) {
		logerr("Cannot open file for write");
		return -1;
	}
	err = _write_statement(current, f);
	fclose(f);
	return err;
}

int _insert_statement(_Statement** cur, Statement stmt) {
	if ( !*cur) {
		int k;
		*cur = (_Statement*)malloc(sizeof(_Statement));
		if ( !*cur) {
			return -200;
		}
		memset(*cur, 0, sizeof(_Statement));
		for( k = 0; k < STATEMENT_SIZE; k++ ) {
			(*cur)->term[k] = wcsdup(stmt[k]);
		}
		return 0;
	} else 	{
		if ( !stmt_match(*cur, stmt) ) {
			// one term differs
			return _insert_statement(&(*cur)->next, stmt);
		}
		// otherwise, the statement already exists
		return 0;
	}
}	

int _remove_statement(_Statement** cur, Statement stmt) {
	if ( *cur) { 
		_Statement** next = &(*cur)->next;
		fwprintf(stderr, L"Removing statement <%s, %s, %s, %s> \n", (*cur)->term[0], (*cur)->term[1], (*cur)->term[2], (*cur)->term[3]);
		if ( !stmt_match(*cur, stmt) ) {
			// at least one term differ -> skip
			return _remove_statement(next, stmt);
		}
		// all terms either masked or equal -> remove
		_stmt_destroy(*cur, 0);		
		*cur = *next;
		return _remove_statement(cur, stmt);
	}
	return 0;
}

int tst_set(TStorage* ts, Statement stmt) {
	return _insert_statement(&ts->statements, stmt); 			

}

int tst_unset(TStorage* ts, Statement stmt) {
	return _remove_statement(&ts->statements, stmt);
}


int _walk_statements(_Statement* cur, Statement mask, StatementResult rs, void* data) {
	if ( cur) {
		int k;

		Statement stmt;
		if ( !stmt_match(cur, mask) ) {
			// at least one term differ -> skip
			return _walk_statements(cur->next, mask, rs, data);
		}
		// make callback
		stmt_extract(cur, &stmt);
		if ( rs(stmt, data) ) {
			return _walk_statements(cur->next, mask, rs, data);
		} else {
			return 1;
		}
	}
	return 0;

}

int tst_get(TStorage* ts, Statement mask, StatementResult rs, void* data) {
	return _walk_statements(ts->statements, mask, rs, data);

}	
int tst_destroy(TStorage* ts) { 
	logerr("Triple storage destroyed");
	_stmt_destroy(ts->statements, 1);
	free(ts);
	return 0;
}

