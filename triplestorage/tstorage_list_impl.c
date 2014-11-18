#include "tstorage.h"


void logerr(const char* err) {
	fprintf(stderr, err);
}

typedef struct _stmt {
	char* term[STATEMENT_SIZE];
	struct _stmt* next;
} _Statement;

struct _tstorage {
	_Statement* statements;
};


int tst_init(TStorage** ts) {
	*ts = (TStorage*)malloc(sizeof(TStorage));
	if (! *ts) 
		return -1;
	(*ts)->statements = NULL;
	return 0;
}

void _stmt_destroy(_Statement* stmt) {
	int k;
	_Statement* next = stmt->next;
	for ( k = 0; k < STATEMENT_SIZE; k++ ){
		free(stmt->term[k]);
	}
	free(stmt);
	if ( next ) 
		_stmt_destroy(next);
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
		ungetc(f, c);
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
		ungetc(f, c);
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
	ungetc(f, c);
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
	if (_read_word(buf, f)) {
		if (_read_qstring(buf, f)) {
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
			_stmt_destroy(*st);
			return -1;
		}
		(*st)->term[k] = strdup(buf);
	}
	return 0;
}

int tst_open(TStorage* ts, const char* filename) {
	FILE* f;
	_Statment** current = &ts->statements;
	f = fopen(filename, "r");
	if ( !f ) {
		logerr("Cannot open file");
		return -1;
	}
		
	while(!feof(f)) {

		if (_read_statement(current, f)) {
			logerr("Cannot read statement");
			return -2;
		}
		current = &(*current)->next;
	}
	return 0;
}

int tst_save(TStorage* ts, const char* filename) {


}

int tst_set(TStorage* ts, Statement stmt);

int tst_unset(TStorage* ts, Statement stmt);
int tst_get(TStorage* ts, Statement mask, StatementResult rs, void* data);
int tst_destroy(TStorage* ts);

