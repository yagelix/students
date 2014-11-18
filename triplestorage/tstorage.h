#ifndef _TSTORAGE_H_
#define _TSTORAGE_H_

#define STATEMENT_SIZE 4

typedef struct _tstorage TStorage;
typedef const char* Statement[STATEMENT_SIZE]; /* c,s,p,o */

typedef int (*StatementResult)(Statement stmt, void* data);


int tst_init(TStorage** ts);
int tst_open(TStorage* ts, const char* filename);
int tst_save(TStorage* ts, const char* filename);
int tst_set(TStorage* ts, Statement stmt);
int tst_unset(TStorage* ts, Statement stmt);
int tst_get(TStorage* ts, Statement mask, StatementResult rs, void* data);
int tst_destroy(TStorage* ts);


#endif /* _TSTORAGE_H_ */
