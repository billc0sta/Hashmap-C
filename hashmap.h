#ifndef HASHMAP_H_
#define HASHMAP_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> 
#define LOAD_FACTOR_MAX 0.6
#define LOAD_FACTOR_MIN 0.1
#define INITIAL_BUCKETS 16
#define MULTIPLY_SPACE 2

#define STATE_UNUSED 0
#define STATE_USED 1
#define STATE_DELETED 2

char _hashmap_last_error[64] = { '\0' };

struct hash_node
{
	void* key;
	void* val;
	struct hash_node* next;
	struct hash_node* prev;
	char state; // 0 = unused, 1 = used, 2 = deleted;
};

typedef struct
{
	struct hash_node* buckets;
	struct hash_node* head;
	struct hash_node* tail;
	void* data;
	size_t mapped;
	size_t key_size;
	size_t val_size;
	size_t space;
	size_t length;
	unsigned int (*hasher)(const void*, unsigned int seed);
	int (*cmp)(const void*, const void*);
	void (*destroy_val)(void*);
	void (*destroy_key)(void*);
	unsigned int seed;
} hashmap;

typedef struct hashmap_iter hashmap_iter; 

hashmap* hashmap_new(size_t key_size, size_t val_size, unsigned int seed,
	unsigned int hasher(const void*, unsigned int), int cmp(const void*, const void*),
	void key_destructor(void*), void value_destructor(void*))
{
	int               mfail   = 0;
	hashmap*          out     = malloc(sizeof(hashmap));
	struct hash_node* buckets = calloc(INITIAL_BUCKETS, sizeof(struct hash_node));
	void*             data    = malloc(key_size * INITIAL_BUCKETS + val_size * INITIAL_BUCKETS);

	if (!out || !buckets || !data)
		mfail = 1;
	if (mfail) 
	{
		if (out) free(out);
		if (data) free(data);
		if (buckets) free(buckets);
		strcpy(_hashmap_last_error, "malloc failed!\n");
		return NULL;
	}
	out->mapped      = 0;
	out->length      = 0;
	out->buckets     = buckets;
	out->data        = data;
	out->key_size    = key_size;
	out->val_size    = val_size;
	out->space       = INITIAL_BUCKETS;
	out->hasher      = hasher;
	out->cmp         = cmp;
	out->destroy_key = key_destructor;
	out->destroy_val = value_destructor;
	out->seed        = seed;
	out->head        = NULL;
	out->tail        = NULL;
	return out;
}

// murmur hash 
static inline unsigned int _murmur_32_scramble(unsigned int k) {
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

unsigned int hashmap_murmur(const void* key, size_t size, unsigned int seed)
{
	unsigned int h = seed;
	unsigned int k;
	for (size_t i = size >> 2; i; i--) {
		memcpy(&k, key, sizeof(unsigned int));
		key = (char*)key + sizeof(unsigned int);
		h ^= _murmur_32_scramble(k);
		h = (h << 13) | (h >> 19);
		h = h * 5 + 0xe6546b64;
	}
	k = 0;
	for (size_t i = size & 3; i; i--) {
		k <<= 8;
		k |= ((char*)key)[i - 1];
	}
	h ^= _murmur_32_scramble(k);
	h ^= size;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

static unsigned int _hash(hashmap* map, void* key)
{
	if (map->hasher) return map->hasher(key, map->seed) % map->space;
	return hashmap_murmur(key, map->key_size, map->seed) % map->space;
}

static int _compare(hashmap* map, void* key1, void* key2)
{
	if (map->cmp) return map->cmp(key1, key2);
	return memcmp(key1, key2, map->key_size);
}

static void _free_val(hashmap* map, void* val)
{
	if (map->destroy_val) map->destroy_val(val);
	free(val);
}

static void _free_key(hashmap* map, void* key)
{
	if (map->destroy_key) map->destroy_key(key);
	free(key);
}

static struct hash_node* _hashmap_find(hashmap* map, void* key)
{
	unsigned int i = _hash(map, key);
	struct hash_node* bucket;
	while ((bucket = &map->buckets[i])->state != STATE_UNUSED && _compare(map, key, bucket->key) != 0)
		i = (i + 1) % map->space;

	return bucket; 
}

// recursive
int hashmap_set(hashmap* map, void* key, void* value);

int hashmap_resize(hashmap* map, size_t resize_by)
{
	if (!map)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	if (resize_by == map->space)
		return 1;

	if (resize_by == 0)
		resize_by = INITIAL_BUCKETS;

	map->             length      = 0;
	map->             mapped      = 0; 
	map->             space       = resize_by;
	struct hash_node* old_head    = map->head;
	struct hash_node* old_buckets = map->buckets;
	void*             old_data    = map->data;
	struct hash_node* new_buckets = calloc(map->space, sizeof(struct hash_node));
	void*             new_data    = malloc(map->key_size * map->space + map->val_size * map->space);
	map->             head        = NULL;
	map->             tail        = NULL;

	int mfail = 0;
	if (!new_buckets || !new_data) mfail = 1;
	if (mfail) 
	{
		if (new_buckets) free(new_buckets);
		if (new_data) free(new_data);
		strcpy(_hashmap_last_error, "malloc failed\n");
		return 0;
	}
	map->buckets = new_buckets;
	map->data = new_data;
	for (struct hash_node* curr = old_head; curr; curr = curr->next) 
		hashmap_set(map, curr->key, curr->val); 

	free(old_buckets);
	free(old_data);

	return 1;
}

int hashmap_set(hashmap* map, void* key, void* value)
{
	if (!map || !key || !value)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	struct hash_node* bucket = _hashmap_find(map, key);
	if (bucket->state != STATE_UNUSED)
	{
		if (map->destroy_val) map->destroy_val(bucket->val);
		memcpy(bucket->val, value, map->val_size);
		bucket->state = STATE_USED;
		return 1;
	}
	bucket->key = (char*)map->data + map->key_size * map->mapped;
	bucket->val = (char*)map->data + map->key_size * map->space + map->val_size * map->mapped;
	++map->mapped;

	memcpy(bucket->key, key, map->key_size);
	memcpy(bucket->val, value, map->val_size);
	bucket->state = STATE_USED;
	bucket->next  = NULL;
	bucket->prev  = map->tail;
	if (!map->head)
	{
		map->head = bucket;
		map->tail = bucket;
	}
	else
	{
		map->tail->next = bucket;
		map->tail = bucket;
	}
	++map->length;

	if ((float)map->length / map->space >= LOAD_FACTOR_MAX)
		hashmap_resize(map, map->space * MULTIPLY_SPACE);

	return 1;
}

void* hashmap_get(hashmap* map, void* key)
{
	if (!map || !key)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return NULL;
	}
	struct hash_node* found = _hashmap_find(map, key);
	if (found->state == STATE_USED) 
		return found->val;

	strcpy(_hashmap_last_error, "no such key!\n");
	return NULL;
}

int hashmap_remove(hashmap* map, void* key)
{
	if (!map || !key)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	struct hash_node* found = _hashmap_find(map, key);
	if (found->state != STATE_USED) 
	{
		strcpy(_hashmap_last_error, "no such key!\n");
		return 0;
	} 

	if (found->prev)
		found->prev->next = found->next;
	if (found->next)
		found->next->prev = found->prev;
	if (found == map->head)
		map->head = found->next;
	if (found == map->tail)
		map->tail = found->prev;

	found->state = STATE_DELETED;
	if (map->destroy_key) map->destroy_key(found->key);
	if (map->destroy_val) map->destroy_val(found->val);
	--map->length;

	if ((float)map->length / map->space <= LOAD_FACTOR_MIN)
		hashmap_resize(map, map->space / MULTIPLY_SPACE);

	return 1; 
}

int hashmap_clear(hashmap* map) 
{
	if (!map)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	for (struct hash_node* curr = map->head; curr; curr = curr->next)
	{
		if (map->destroy_key) map->destroy_key(curr->key);
		if (map->destroy_val) map->destroy_val(curr->val);
		curr->state = STATE_UNUSED;
	}
	map->head = NULL;
	map->tail = NULL;
	map->mapped = 0;
	map->length = 0;
	return 1;
} 

int hashmap_free(hashmap* map)
{
	if (!map)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	hashmap_clear(map);
	free(map->buckets);
	free(map->data);
	free(map);
	return 1;
}

hashmap_iter* hashmap_iterator(hashmap* map)
{
	if (!map) 
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return NULL; 
	}
	return (hashmap_iter*)map->head;
}

int hashmap_next(hashmap_iter** iter, void** key, void** val)
{
	if (!iter || !key || !val)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	if (!*iter)
		return 0;

	struct hash_node* node = (struct hash_node*)*iter;
	*key  = node->key;
	*val  = node->val;
	*iter = (hashmap_iter*)node->next; 
	return 1;
}

int hashmap_scan(hashmap* map, void iter_func(void*, void*))
{
	if (!map)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	if (map->length == 0)
	{
		strcpy(_hashmap_last_error, "empty map!\n");
		return 0;
	}
	hashmap_iter* iter = hashmap_iterator(map);
	void* key = NULL;
	void* val = NULL;
	while (hashmap_next(&iter, &key, &val))
	{
		iter_func(key, val);
	}
	return 1;
}

const char* hashmap_error()
{
	return _hashmap_last_error;
}

size_t hashmap_count(hashmap* map)
{
	if (!map)
	{
		strcpy(_hashmap_last_error, "passed null pointer!\n");
		return 0;
	}
	return map->length;
}
#endif