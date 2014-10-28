#include"hash.h"
#include"stdio.h"
#include"string.h"

struct _htable {
	size_t size;
	void** array;
	char* deleted;
	ht_equal_func equal;
	ht_hash_func hf;
	ht_clone_func cf;
	ht_destroy_func df;
	size_t elements;
};

int htable_create(HTable** t, size_t initial_size,
		ht_hash_func hf,
		ht_equal_func ef,
		ht_clone_func cf,
		ht_destroy_func df){
	*t = (HTable*)malloc(sizeof(HTable));
	if (!*t)
		return -1;
	
	(*t)->array = (void**)malloc(sizeof(void*)*initial_size);
	if (!(*t)->array) 
		return -2;
	(*t)->deleted = (char*)malloc(sizeof(char)*(initial_size>>3 + 1));
	if ( !(*t)->deleted) 
		return -3;
	
	memset((*t)->array, 0, sizeof(void*)*initial_size);
	memset((*t)->deleted, 0, sizeof(char)*(initial_size >> 3 + 1));
	(*t)->hf = hf;
	(*t)->cf = cf;
	(*t)->equal = ef;
	(*t)->df = df;
	(*t)->size = initial_size;
	(*t)->elements = 0;
	return 0;
}

void htable_destroy(HTable* t) {
	int i;
	for (i = 0; i < t->size; i++ ) {
		if (t->array[i]) 
			t->df(t->array[i]);
	}
	free(t->deleted);
	free(t->array);
	free(t);
}




int _is_set(char* bmp, size_t i) {
	char byte = bmp[i/8];
	return byte & (1 << (i % 8));
}

void _bmp_set(char *bmp, size_t i) {
	char *byte = bmp + (i>>3);
	*byte |= (1 << (i & 0x7));
}

void _ensure_capacity(HTable* t) {
	if ( t->elements*10 > t->size*7) {
		size_t i,k;
		size_t new_sz = t->size * 2;
		void** new_array = (void**)malloc(sizeof(void*)*new_sz);
		char* new_deleted = (char*)malloc(sizeof(char)*(new_sz>>3 + 1));
		if (! new_array) 
			return;
		if ( ! new_deleted) 
			return;
		memset(new_array, 0, sizeof(void*)*new_sz);
		memset(new_deleted, 0, sizeof(char)*(new_sz>>3 + 1));
		for(i = 0; i < t->size; i++ ) {
			if ( t->array[i] ) {
				if ( !_is_set(t->deleted, i)) {
					for(k = 0; k < new_sz; k++) {
						size_t idx = (t->hf(t->array[i], new_sz) + k) % new_sz;
						if( new_array[idx] == NULL ) {
							new_array[idx] = t->array[i];
							break;
						}
					}
				} else {
					t->elements--;
					t->df(t->array[i]);
				}
			}
		}
		free(t->array);
		free(t->deleted);
		t->array = new_array;
		t->deleted = new_deleted;
		t->size = new_sz;
	}
}


int htable_insert(HTable* t, void* data, int replace) {
	size_t hv = t->hf(data, t->size);
	int i;
	for ( i = 0; i < t->size; i++) {
		size_t idx = (hv + i) % t->size;
		if (t->array[idx] == NULL ) {
			t->array[idx]  = t->cf(data);
			t->elements++;
			break;
		} else if ( t->equal(t->array[idx], data) ) {
			if (replace || _is_set(t->deleted, idx) ) {
				t->df(t->array[idx]);
				t->array[idx] = t->cf(data);
			} 
			break;
		}
	}
	_ensure_capacity(t);
	return 0;
}

int htable_remove(HTable* t, void* data) {
	size_t hv = t->hf(data, t->size);
	size_t k;
	for(k = 0; k < t->size; k++) {
		size_t idx = (hv + k) % t->size;
		if ( t->array[idx] ) {
			if (t->equal(t->array[idx], data)){
				_bmp_set(t->deleted, idx);
				return 0;
			}
		}
	}
	return -1;
}

size_t htable_get_size(HTable* t) {
	return t->size;
}

size_t htable_get_capacity(HTable* t) {
	return t->size - t->elements;
}

int htable_get(HTable* t, void* key, void** data) {
	size_t hv = t->hf(key, t->size);
	size_t i;
	for(i = 0; i < t->size; i++ ) {
		size_t idx = (hv + i) % t->size;
		if ( t->array[idx] ) {
			if (t->equal(t->array[idx], key) && !_is_set(t->deleted, idx)) {
				*data = t->array[idx];
				return 0;
			}
		} else {
			return -1;
		}
	}
	return -1;
}


