#ifndef HASHMAP
#define HASHMAP
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
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
} Hashmap;

// initializes a hashmap and returns it
Hashmap HS_create_hashmap(uint32_t key_typesize, uint32_t val_typesize);
// deallocates the hashmap resources
int HS_destroy_hashmap(Hashmap *hashmap);
// adds element by key
int HS_add_element(Hashmap *hashmap, void *key, void *val);
// removes element by key
int HS_remove_element(Hashmap *hashmap, void *key);
// applies a function to each element in the hashmap
void HS_traverse_hashmap(Hashmap *hs, void func(void *, void *));
// finds a value by key if exists, returns NULL otherwise
void *HS_get_element(Hashmap *hs, void *key);
// gets the value of _reserved
uint64_t HS_get_reserved(Hashmap *hs);
// gets the value of _count
uint64_t HS_get_count(Hashmap *hs);
// reserves more buckets
int HS_reserve(Hashmap *hs, uint64_t items);
// sets the comparison function for ordered mode
void HS_clear_hashmap(Hashmap *hs);
// returns the last item in list
void *HS_get_head(Hashmap *hs);
// applies a function to each element in hashmap + additional parameter can be
// used as an accumulator
void HS_traverse_hashmap_payload(Hashmap *hs, void func(void *, void*, void*), void *payload);


#endif
