#ifndef HASHMAP
#define HASHMAP
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h> // @Testing
#define min(f, s) (f < s) ? f : s
#define BASE_SIZE 50

struct _HS_Node {
  void *key;
  void *val;
  struct _HS_Node *next;
  struct _HS_Node *prev;
  bool is_used;
  bool is_parent;
  bool is_child;
};
typedef struct {
  uint64_t _count;
  uint32_t _key_typesize;
  uint32_t _val_typesize;
  uint64_t _reserved;
  struct _HS_Node **_arr;
  struct _HS_Node *_tail;
  struct _HS_Node *_head;
  FILE *_errstream;
} Hashmap;

// sets a node to it's default values
static void _HS_clear_node(struct _HS_Node *nd);
// deallocates data
static inline void _HS_free_data(void *key, void *val);
// removes linkage and deallocates a node
static inline void _HS_remove_node(struct _HS_Node *nd);
// inserts a node in front of another, assumes back is disowned
static inline void _HS_insert_next(struct _HS_Node *front, struct _HS_Node *back);
// reports an error to the stderr
static inline void _HS_report_error(FILE *errstream, const char *error, const char *reason);
// murmur hashing function
static uint32_t _HS_hash(unsigned char *key, uint64_t key_typesize);
// adds a node to the end of the list
static void _HS_add_end(Hashmap *hs, struct _HS_Node *nd);
// returns list node by key if exists, returns NULL otherwise
static struct _HS_Node* _HS_get_node(Hashmap *hs, void *key);
// reserves more memory for _arr
static void _HS_reserve_more(Hashmap *hs, uint64_t size);

Hashmap HS_create_hashmap(uint32_t key_typesize, uint32_t val_typesize) {
  Hashmap hs;
  hs._errstream = NULL;
  if (key_typesize == 0 || val_typesize == 0) {
    _HS_report_error(hs._errstream, "create_hashmap() failed", "invalid type sizes");
    return hs;
  }
  hs._reserved = BASE_SIZE;
  hs._arr = calloc(hs._reserved, sizeof(struct _HS_Node *));
  if (!hs._arr) {
    _HS_report_error(hs._errstream, "create_hashmap() failed", "allocation failed");
    return hs;
  }  
  hs._key_typesize = key_typesize;
  hs._val_typesize = val_typesize;
  hs._count = 0;
  hs._head = NULL;
  hs._tail = NULL;
  return hs;
}

void set_hashmap_errstream(Hashmap *hs, FILE *stream) {
  if (!hs || !stream) {
    _HS_report_error(hs->_errstream, "set_hashmap_errstream() failed", "invalid NULL ptr");
  }
  hs->_errstream = stream;
}

uint64_t HS_get_reserved(Hashmap *hs) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "get_reserved() failed", "invalid NULL ptr");
    return 0;
  }
  return hs->_reserved;
}

uint64_t HS_get_count(Hashmap *hs) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "get_count() failed", "invalid NULL ptr");
    return 0;
  }
  return hs->_count;
}

int HS_reserve(Hashmap *hs, uint64_t size) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "reserve() failed", "invalid NULL ptr");
    return -1;
  }
  _HS_reserve_more(hs, size);
  return 0;
}

void HS_clear_hashmap(Hashmap *hs) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "clear_hashmap() failed", "invalid NULL ptr");
    return;
  }
  struct _HS_Node *current = hs->_head;
  for (int i = 0; i < hs->_count; ++i) {
    _HS_free_data(current->key, current->val);
    struct _HS_Node *next = current->next;
    free(current);
    current = next;
  }
  hs->_key_typesize = 0;
  hs->_val_typesize = 0;
  hs->_count = 0;
  hs->_tail = NULL;
  hs->_head = NULL;
  hs->_errstream = NULL;
}

int HS_destroy_hashmap(Hashmap *hs) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "destroy_hashmap() failed", "invalid NULL ptr");
    return -1;
  }
  // zeroing and deallocating nodes
   HS_clear_hashmap(hs);
  hs->_reserved = 0;
  
  // deallocate the array
  free(hs->_arr);
  hs->_arr = NULL;  
  return 0;
}

void HS_traverse_hashmap(Hashmap *hs, void func(void * key, void * val)) {
  if (!hs || !func) {
    _HS_report_error(hs->_errstream, "traverse_hashmap() failed", "invalid NULL ptr");
    return;
  }
  if (hs->_count == 0) {
    _HS_report_error(hs->_errstream, "traverse_hashmap() failed",
                     "no elements in hashmap");
    return;
  }
  struct _HS_Node *current = hs->_head->next; // to start by first added
  for (int i = 0; i < hs->_count; ++i, current = current->next) {
    func(current->key, current->val);
  }
}

void HS_traverse_hashmap_payload(Hashmap *hs,
                                 void func(void *key, void *val, void *payload),
                                 void *payload) {
  if (!hs || !func) {
    _HS_report_error(hs->_errstream, "traverse_hashmap_payload() failed", "invalid NULL ptr");
    return;
  }
  if (hs->_count == 0) {
    _HS_report_error(hs->_errstream, "traverse_hashmap_payload() failed",
                     "no elements in hashmap");
    return;
  }
  struct _HS_Node *current = hs->_head->next;
  for (int i = 0; i < hs->_count; ++i, current = current->next) {
    func(current->key, current->val, payload);
  }  
}

void *HS_get_head(Hashmap *hs) {
  if (!hs) {
    _HS_report_error(hs->_errstream, "get_head() failed", "invalid NULL ptr");
    return NULL;
  }
  if (!hs->_head) {
    _HS_report_error(hs->_errstream, "get_head() failed", "no elements in hashmap");
    return NULL;
  }
  return hs->_head;
}

void *HS_get_element(Hashmap *hs, void *key) {
  if (!hs || !key) 
    return NULL;

  uint32_t hash = _HS_hash((unsigned char *)key, hs->_key_typesize) % hs->_reserved;
  struct _HS_Node *nd = hs->_arr[hash];

  if (!nd || !nd->is_used)
    return NULL;

  if (memcmp(nd->key, key, hs->_key_typesize) == 0) {
    return nd->val;
  } else {
    nd = nd->next;
    while (nd->is_child) {
      if (memcmp(nd->key, key, hs->_key_typesize) == 0) {
	return nd->val;
      }
    }
    return NULL;
  }
}

int HS_add_element(Hashmap *hs, void *key, void *val) {
  if (!hs || !key || !val) {
    _HS_report_error(hs->_errstream, "add_element() failed", "invalid NULL ptr");
    return -1;
  }
  if (hs->_count == hs->_reserved - 1) {
    _HS_reserve_more(hs, BASE_SIZE);
  }
  uint64_t ktsize = hs->_key_typesize;
  uint64_t vtsize = hs->_val_typesize;

  struct _HS_Node *nd = calloc(1, sizeof(struct _HS_Node));
  nd->key = malloc(ktsize);
  nd->val = malloc(vtsize);
  if (!nd || !nd->key || !nd->val) {
    if (nd) free(nd);
    if (nd->key) free(nd->key);
    if (nd->val) free(nd->val);
    _HS_report_error(hs->_errstream, "add_element() failed", "allocation failed");
    return -1;
  }
  memcpy(nd->key, key, ktsize);
  memcpy(nd->val, val, vtsize);
  nd->is_used = 1;

  uint32_t hash = _HS_hash((unsigned char *)key, ktsize) % hs->_reserved;
  
  if (!hs->_arr[hash]) {
    hs->_arr[hash] = nd;
    _HS_add_end(hs, nd);
    ++hs->_count;
    return 0;
  } else {
    void *intrin = HS_get_element(hs, key); // value ptr
    if (intrin) {
      _HS_remove_node(nd);
      memcpy(intrin, val, vtsize);
      return 0;
    }
  }
  // no identical key exists, but the bucket is filled
  // that means one thing
  nd->is_child = 1;
  _HS_insert_next(hs->_arr[hash], nd);
  if (hs->_arr[hash]->is_parent) {
      nd->next->prev = nd;
      nd->is_parent = 1;
  }
  hs->_arr[hash]->is_parent = 1;
  ++hs->_count;
  return 0;
}

int HS_remove_element(Hashmap *hs, void *key) {
  printf("hello there");
  if (!hs || !key) {
    _HS_report_error(hs->_errstream, "remove_element() failed",
                     "invalid NULL ptr");
    return -1;
  }
  struct _HS_Node *nd = _HS_get_node(hs, key);  
  if (!nd) {
    _HS_report_error(hs->_errstream, "remove_element() failed", "no such key");
    return -1;
  }

  struct _HS_Node *next = nd->next; // possibly a child
  int is_child = nd->is_child;
  int is_parent = nd->is_parent;
  
  if (nd == hs->_head)
    hs->_head = nd->prev;
  if (nd == hs->_tail)
    hs->_tail = nd->next;

  _HS_remove_node(nd);
  // replace by child
  if (is_parent && !is_child) {
    uint32_t hash = _HS_hash((unsigned char *)key, hs->_key_typesize) % hs->_reserved;
    hs->_arr[hash] = next;
  }
  --hs->_count;
  if (hs->_count == 0) hs->_head = NULL;
  return 0;
}

static inline void _HS_free_data(void *key, void *val) {
  if (!key || !val)
    return;
  
  free(val);
  free(key);
}

static inline uint32_t _HS_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

static uint32_t _HS_hash(unsigned char *key, uint64_t key_size) {
  uint32_t h = 0x85ebca6b;
  uint32_t k; 
  for (uint64_t i = key_size >> 2; i; --i) {
    memcpy(&k, key, sizeof(uint32_t));
    key += sizeof(uint32_t);
    h ^= _HS_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }
  k = 0;
  for (uint64_t i = key_size & 3; i; --i) {
    k <<= 8;
    k |= key[i - 1];
  }
  h ^= _HS_scramble(k);
  h ^= key_size;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

// possibly unused
static void _HS_clear_node(struct _HS_Node *nd) {
  if (!nd)
    return;
  
  nd->is_used = 0;
  nd->is_parent = 0;
  nd->is_child = 0;
  nd->val = NULL;
  nd->key = NULL;
  nd->next = NULL;
  nd->prev = NULL;
}

static void _HS_reserve_more(Hashmap *hs, uint64_t size) {
  if (!hs)
    return;
  free(hs->_arr);
  hs->_arr = calloc(hs->_reserved + size, sizeof(struct _Node *));
  if (!hs->_arr) {
    _HS_report_error(hs->_errstream, "resizing hashmap failed", "allocation failed");
    return;
  }
  hs->_reserved += size;
  struct _HS_Node *current = hs->_head;
  
  for (int i = 0; i < hs->_count; ++i, current = current->next) {
    current->is_child  = 0;
    current->is_parent = 0;
    uint32_t hash = _HS_hash((unsigned char*)current->key, hs->_key_typesize) % hs->_reserved;
    
    if (!hs->_arr[hash]) {
      hs->_arr[hash] = current;
      continue;
    }
    
    current->is_child = 1;
    _HS_insert_next(current, hs->_arr[hash]);
    if (hs->_arr[hash]->is_parent) {
      current->next->prev = current;
      current->is_parent = 1;
    }
    hs->_arr[hash]->is_parent = 1;
  }
}

static inline void _HS_report_error(FILE *errstream, const char *error,
                                    const char *reason) {
  if (errstream == NULL)
    errstream = stderr;
  fprintf(stderr, "\n[HS error]: %s \nreason: %s\n", error, reason);
}

static inline struct _HS_Node *_HS_get_node(Hashmap *hs, void *key) {
  if (!hs || !key)
    return NULL;
  
  uint32_t hash = _HS_hash((unsigned char*)key, hs->_key_typesize) % hs->_reserved;
  struct _HS_Node *nd = hs->_arr[hash];

  if (!nd || !nd->is_used)
    return NULL;

  if (memcmp(nd->key, key, hs->_key_typesize) == 0) {
    return nd;
  } else {
    nd = nd->next;
    while (nd->is_child) {
      if (memcmp(nd->key, key, hs->_key_typesize) == 0) {
	return nd;
      }
    }
    return NULL;
  }

}

static inline void _HS_insert_next(struct _HS_Node *back, struct _HS_Node *front) {
  if (!back || !front)
    return;
  
  front->next = back->next;
  front->prev = back;
  back->next = front;
}

static inline void _HS_remove_node(struct _HS_Node *nd) {
  if (!nd)
    return;
  nd->prev->next = nd->next;
  nd->next->prev = nd->prev;
  _HS_free_data(nd->key, nd->val);
  free(nd);
}

static void _HS_add_end(Hashmap *hs, struct _HS_Node *nd) {
  if (!hs || !nd)
    return;
  if (!hs->_head || !hs->_tail) {
    nd->next  = nd;
    nd->prev  = nd;
    hs->_tail = nd;
    hs->_head = nd;
  }
  hs->_head->next = nd;
  hs->_tail->prev = nd;
  nd->prev = hs->_head;
  nd->next = hs->_tail;
  hs->_head = nd;
}

// @TODO: add prime distribution
// @TODO: add string hashing
// @TODO: add iterator
#endif
