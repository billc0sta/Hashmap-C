# C Hashmap Implementation# C Hashmap Implementation
simple and fast C hashmap implementation.

## Usage
```C
int main() {
  // set logging stream
  errstream = stderr;

  int key;
  struct MyType {
    int f;
    int s;
  } val;
  // initialize
  Hashmap hs = HS_create_hashmap(sizeof(key), sizeof(val));

  // add
  key = 132;
  val.f = 100;
  val.s = 32;
  HS_add_element(&hs, &key, &val);

  // get
  MyType *my_val = HS_get_element(&hs, key);
  if (!my_val) {
    printf("no such element");
  } else {
    // use
    printf("f: %d\n, s: %d\n", my_val->f, my_val->s);
  }

  // deallocate hashmap resources
  HS_destroy_hashmap(&hs);
  return 0;
}
```

## functions
### basic
```C
// intializes a hashmap and returns it
Hashmap HS_create_hashmap(uint32_t key_size, uint32_t val_size);

// deallocates the hashmap resources
int HS_destroy_hashmap(Hashmap *hashmap);

// adds element by key
int HS_add_element(Hashmap *hashmap, void *key, void *val);

// removes element by key
int HS_remove_element(Hashmap *hashmap, void *key);

// finds a value by key if exists, returns NULL otherwise
void *HS_get_element(Hashmap *hs, void *key);
```
### memory
```C
// reserves more memory buckets
int HS_reserve(Hashmap *hs, uint64_t items);

// sets hashmap fields to zero and deallocates all pairs
void HS_clear_hashmap(Hashmap *hs);
```

### iterating
```C
// applies a function to each element in the hashmap
void HS_traverse_hashmap(Hashmap *hs, void func(void *, void *));

// applies a function to each element in hashmap + additional parameter can be
// used as an accumulator
void HS_traverse_hashmap_payload(Hashmap *hs, void func(void *, void*, void*), void *payload);
```

### additional
```C
// returns how many buckets are available
uint64_t HS_get_reserved(Hashmap *hs);

// returns how many pairs are in the hashmap
uint64_t HS_get_count(Hashmap *hs);

// returns the last added element, returns NULL if hashmap is empty
void *HS_get_head(Hashmap *hs);
```
