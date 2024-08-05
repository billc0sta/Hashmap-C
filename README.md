simple and acceptably fast C hashmap implementation.
## Features
- header only
- very easy to use
- pretty good performance
- generic interface allowing variable sized keys

## Usage
```C
unsigned int hash_string(const void* str, unsigned int seed)
{
	char* s = *(char**)str;
	return hashmap_murmur(s, strlen(s), seed);
}

int compare_string(const void* str1, const void* str2)
{
	const char* s1 = *(char**)str1;
	const char* s2 = *(char**)str2;
	return strcmp(s1, s2);
}

int sum = 0;
void callback_scan(void* key, void* val)
{
	sum += *(int*)val;
}

int main()
{ 
	hashmap* map = hashmap_new(sizeof(char*), sizeof(int), 0, hash_string, compare_string, NULL, NULL);

	hashmap_set(map, &(char*){"one"}, &(int){1});
	hashmap_set(map, &(char*){"two"}, &(int){2});
	hashmap_set(map, &(char*){"three"}, &(int){3});
	hashmap_set(map, &(char*){"four"}, &(int){4});

	int* two = hashmap_get(map, &(char*){"two"});
	if (!two)
		printf("hashmap error: %s\n", hashmap_error());
	else
		printf("two: %d\n", *two);

	hashmap_remove(map, &(char*){"four"});
	printf("count: %zu\n", hashmap_count(map));

	size_t i = 0;
	void* key;
	void* val;
	while (hashmap_next(map, &i, &key, &val))
	{
		char* k = *(char**)key;
		int v = *(int*)val;
		printf("{key -> %s : val -> %d}\n", k, v);
	}

	hashmap_scan(map, callback_scan);
	printf("sum: %d\n", sum);
	hashmap_free(map);
}
```

## Functions
### Basic
```
-> hashmap_new      // allocates a new hashmap
-> hashmap_set      // sets a new pair, replaces value if key exists
-> hashmap_get      // returns the value if key exists, NULL otherwise
-> hashmap_remove   // removes pair if key exists, does nothing otherwise
-> hashmap_clear    // clears the hashmap without deallocation, the hashmap can be used again
-> hashmap_free     // frees the hashmap and it's allocated buckets, the hashmap can't be used again
```
### Iterators
```
-> hashmap_next     // advances the allocator and fills the key and value parameters
-> hashmap_scan     // applies a callback function for each key/value pair;
```
### Utilities
```
-> hashmap_murmur   // hashing function for hashing callback funcs
-> hashmap_error    // returns the last error;
-> hashmap_count    // returns the current length of the hashmap;
-> hashmap_resize   // resizes the hashmap (not recommended to use manually)
```

## Benchmarking
```
hashmap_add    - 5000000 test cases took: 1.405000 sec
hashmap_get    - 5000000 test cases took: 0.615000 sec
hashmap_scan   - 5000000 test cases took: 0.037000 sec
hashmap_remove - 5000000 test cases took: 0.876000 sec
```
