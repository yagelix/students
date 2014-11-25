#include "tstorage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

void logerr(const char* err, ...) {
	va_list ap;
	va_start(ap, err);
	vfprintf(stderr, err, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

typedef struct _stmt {
	char* term[STATEMENT_SIZE];
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
		(*stmt)[k] = va_arg(ap, char *);
	}
	va_end(ap);
}

int stmt_match(_Statement* stmt, Statement mask) {
	int k;
	for (k = 0; k < STATEMENT_SIZE; k++ ){
		if ( mask[k] && strcmp(stmt->term[k], mask[k])) 
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

int _read_word(char* buf, FILE* f) {
	long pos = ftell(f);
	while (!feof(f)) {
		int c = fgetc(f);
		if ( isalnum(c) ) {
			*buf = c;
			buf++;
		} else if (isspace(c)) {
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
	if ( feof(f) ){
		return -1;
	}
	if ( fgetc(f) == '\"') {
		return 0;
	}
	fseek(f, pos, SEEK_SET);
	return -2;

}

int _read_nonescaped(char** buf, FILE* f) {
	int c = fgetc(f);
	if ( c == '\\' ) {
		ungetc(c, f);
		return  -1;
	} else {
		**buf = c;
		(*buf)++;
	}
	return 0;
}

int _read_escaped(char** buf, FILE* f) {
	int c = fgetc(f);
	if ( feof(f) ) {
		return -2;
	}
	if( c != '\\') {
		ungetc(c, f);
		return -1;
	}
	
	c = fgetc(f);
	if ( feof(f)) {
		return -3;
	}

	switch (c) {
	case 't': 
		**buf = '\t';
		break;
	case 'n': 
		**buf = '\n';
		break;
	case 'r':
		**buf = '\r';
		break;
	default:
		**buf = c;
	}
	(*buf)++;
	return 0;
}

int _read_until_quote(char* buf, FILE* f) {
	
	int c;
	long pos = ftell(f);
	c = fgetc(f);
	if (c == '\"') 
		return 0;
	ungetc(c, f);
	if ( _read_nonescaped(&buf, f) ) {
		if ( _read_escaped(&buf, f) ) {
			fseek(f, pos, SEEK_SET);
			return -9;
		}
	}
	return _read_until_quote(buf, f);
	
}

int _read_qstring(char* buf, FILE* f) {
	long pos = ftell(f);
	if (_read_quote(f)) 
		return -6;
	if (_read_until_quote(buf, f)) {
		fseek(f, pos, SEEK_SET);
		return -7;
	}
	return 0;

}


int _read_term(char* buf, FILE* f) {
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
	char buf[4096];
	*st = (_Statement*)malloc(sizeof(_Statement));
	memset(*st, 0, sizeof(_Statement));

	for (k = 0; k < STATEMENT_SIZE; k++ ){
		if ( _read_term(buf, f) ) {
			logerr("Cannot read term at pos %ld", ftell(f));
			_stmt_destroy(*st, 1);
			return -1;
		}
		(*st)->term[k] = strdup(buf);
	}
	return 0;
}

int tst_open(TStorage* ts, const char* filename) {
	FILE* f;
	_Statement** current = &ts->statements;

	f = fopen(filename, "r");

	if ( !f ) {
		logerr("Cannot open file");
		return -1;
	}
		
	while(!feof(f)) {
		if (_read_statement(current, f)) {
			logerr("Cannot read statement at pos %ld", ftell(f));
			return -2;
		}
		current = &(*current)->next;
	}
	return 0;
}

int _write_term_escaped(const char* term, FILE* f) {
	const char *k;
	fprintf(f, "\"");
	for(k = term; *k; k++ ) {
		if ( *k == '\\' || *k == '\"' ) {
			fprintf(f, "\\%c", *k);	
		} else if (*k == '\n' ) {
			fprintf(f, "\\n");
		} else if (*k == '\r') {
			fprintf(f, "\\r");
		} else {
			fprintf(f, "%c", *k);
		}
	}
	fprintf(f, "\"");
	return 0;
		
}

int _write_term(const char* term, FILE* f) {
	const char* k;
	for (k = term; *k; k++ ) {
	     if (isspace(*k) || *k == '\\' || *k=='\"' || *k == '\'') {

			return _write_term_escaped(term, f);       		       
	     }
    }
	if (fprintf(f, term) != strlen(term)) {
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
				if ( fprintf(f, " ") != 1)  {
					logerr("Cannot write whitespace");
					return -5;
				}
			}

			if ( _write_term(stmt->term[i], f)){
				logerr("Cannot write term %d", i);
				return -1;
			}
		}
		if ( fprintf(f, "\n") != 1) {
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
			(*cur)->term[k] = strdup(stmt[k]);
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

