simple and acceptably fast C hashmap implementation.

## Usage
```C
void free_string(void* str)
{
	// this works similarly to a C++ destructor.
	// don't free the pointer itself, only free and destruct your data.
	free(*(char**)str);
}

unsigned int hash_string(const void* str, unsigned int seed) 
{
	return hashmap_murmur(str, strlen(str), seed);
}

int compare_string(const void* str1, const void* str2) 
{
	const char* s1 = str1;
	const char* s2 = str2;
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

	hashmap_set(map, &"one", &(int){1});
	hashmap_set(map, &"two", &(int){2});
	hashmap_set(map, &"three", &(int){3});
	hashmap_set(map, &"four", &(int){4});

	int* two = hashmap_get(map, "two");
	if (!two)
		printf("hashmap error: %s\n", hashmap_error());
	else
		printf("two: %d\n", *two);

	hashmap_remove(map, &"four");
	printf("count: %zu\n", hashmap_count(map)); 

	hashmap_iter* iter = hashmap_iterator(map);
	void* key;
	void* val;
	while (hashmap_next(&iter, &key, &val)) 
	{
		char* k = key;
		int v = *(int*)val;
		printf("{key -> %s : val -> %d}\n", k, v);
	} 

	hashmap_scan(map, callback_scan);
	printf("sum: %d\n", sum);

	hashmap_free(map);
}
```

## functions
```
-> hashmap_new      // allocates a new hashmap
-> hashmap_set      // sets a new pair, replaces value if key exists
-> hashmap_get      // returns the value if key exists, NULL otherwise
-> hashmap_remove   // removes pair if key exists, does nothing otherwise
-> hashmap_murmur   // hashing function for hashing callback funcs
-> hashmap_resize   // resizes the hashmap (not recommended to use manually)
-> hashmap_clear    // clears the hashmap without deallocation, the hashmap can be used again
-> hashmap_free     // frees the hashmap and it's allocated buckets, the hashmap can't be used again
-> hashmap_iterator // creates and returns an iterator pointer (doesn't need to be freed)
-> hashmap_next     // advances the allocator and fills the key and value parameters
-> hashmap_scan     // applies a callback function for each key/value pair;
-> hashmap_error    // returns the last error;
-> hashmap_count    // returns the current length of the hashmap;

```

## benchmarking
```
hashmap_add    - 5000000 test cases took: 2.222000 sec
hashmap_get    - 5000000 test cases took: 0.587000 sec
hashmap_scan   - 5000000 test cases took: 0.442000 sec
hashmap_remove - 5000000 test cases took: 1.159000 sec
```
