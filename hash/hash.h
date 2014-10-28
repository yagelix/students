#ifndef __HASH_H__
#define __HASH_H__

typedef struct _htable HTable;
typedef size_t (*ht_hash_func)(void* data, size_t sz);
typedef void* (*ht_clone_func)(void* data);
typedef void (*ht_destroy_func)(void* data);
typedef int (*ht_equal_func)(void* a, void* b); /* returns true  if a equals b*/

int htable_create(HTable** t, size_t initial_size,
		ht_hash_func hf,
		ht_equal_func ef,
		ht_clone_func cf,
		ht_destroy_func df);

void htable_destroy(HTable* t);

int htable_insert(HTable* t, void* data, int replace);

int htable_remove(HTable* t, void* data);

size_t htable_get_size(HTable* t);
size_t htable_get_capacity(HTable* t);

int htable_get(HTable* t, void* key, void** data);


#endif /* __HASH_H__ */
