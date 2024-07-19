#include "hashmap.h"
#include <stdio.h>
#include <time.h>

void report_error(const char* func_name, size_t test_case)
{
	fprintf(stderr,
		"something happened at: %s()\n"
		"test case:% zu\n"
		"error: %s\n",
		func_name, test_case, hashmap_error());
}

double benchmark_set(hashmap* map, size_t test_cases)
{
	clock_t begin = clock();
	for (size_t i = 0; i < test_cases; ++i)
	{
		if (!hashmap_set(map, &i, &i))
		{
			report_error("test_set", i);
			return 0;
		}
	}
	clock_t end = clock();
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

double benchmark_get(hashmap* map, size_t test_cases)
{
	clock_t begin = clock();
	for (size_t i = 0; i < test_cases; ++i)
	{
		if (!hashmap_get(map, &i))
		{
			report_error("test_get", i);
			return 0;
		}
	}
	clock_t end = clock();
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

double benchmark_remove(hashmap* map, size_t test_cases)
{
	clock_t begin = clock();
	for (size_t i = 0; i < test_cases; ++i)
	{
		if (!hashmap_remove(map, &i))
		{
			report_error("test_remove", i);
			return 0;
		}
	}
	clock_t end = clock();
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

size_t map_length = 0;
void count_length(void* key, void* val)
{
	map_length += 1;
}

double benchmark_scan(hashmap* map, size_t test_cases)
{
	clock_t begin = clock();
	hashmap_scan(map, count_length);
	if (map_length != test_cases)
	{
		printf("%zu\n", map_length);
		report_error("test_scan", 0);
	}
	map_length = 0;
	clock_t end = clock();
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

void benchmark_all(hashmap* map, size_t test_cases) 
{
	printf("testing started\n");
	printf("hashmap_add    - %zu test cases took: %f sec\n", test_cases, benchmark_set(map, test_cases));
	printf("hashmap_get    - %zu test cases took: %f sec\n", test_cases, benchmark_get(map, test_cases));
	printf("hashmap_scan   - %zu test cases took: %f sec\n", test_cases, benchmark_scan(map, test_cases));
	printf("hashmap_remove - %zu test cases took: %f sec\n", test_cases, benchmark_remove(map, test_cases));
}

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