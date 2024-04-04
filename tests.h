int test_HSCreateHashmap();
int test_HSDestroyHashmap();
int test_HSAddElement();
int test_HSRemoveElement();
int test_HSGetElement();
void sum(void *key, void *val, void *payload);
int test_HSTraverse();
int test_HSReserve();
void benchmark();

#include "hashmap.h"
#include <stdio.h>

int test_HSCreateHashmap() {
  // should end program
  // Hashmap hs1 = HS_create_hashmap(0, 1);
  Hashmap hs2 = HS_create_hashmap(10, 10);
  int test1 = hs2._key_typesize == 10 && hs2._val_typesize == 10 &&
              hs2._reserved == BASE_SIZE;
  
  HS_destroy_hashmap(&hs2);
  return test1;
}

int test_HSDestroyHashmap() {
  // should end program
  // HS_destroy_hashmap(NULL);
  Hashmap hs = HS_create_hashmap(1, 1);
  HS_destroy_hashmap(&hs);
  int test1 = hs._reserved == 0 && hs._key_typesize == 0 &&
              hs._val_typesize == 0 && hs._arr == NULL && hs._count == 0;

  return test1;
}

int test_HSAddElement() {
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key;
  uint64_t val;
  int ires;
  key = 'A';
  val = 123456789;

  ires = HS_add_element(NULL, NULL, NULL);
  int test1 = ires == -1;
  ires = HS_add_element(&hs, &key, &val);
  
  void *old_last_added = hs._head;
  int test2 = ires == 0 && hs._count == 1 && hs._head != NULL &&
              *(uint64_t *)HS_get_element(&hs, &key) == val;

  key = 'B';
  val = 103232;
  ires = HS_add_element(&hs, &key, &val);
  int test3 = ires == 0 && hs._count == 2 && hs._head != old_last_added &&
              *(uint64_t *)HS_get_element(&hs, &key) == val;
 
  HS_destroy_hashmap(&hs);
  return test1 && test2 && test3;
}

int test_HSRemoveElement() {
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key;
  uint64_t val;
  int ires;
  key = 'A';
  val = 1;

  printf("before!\n");
  ires = HS_remove_element(NULL, NULL);
  printf("afterasdasd\n");
  int test1 = ires == -1;
  // key not added yet
  ires = HS_remove_element(&hs, &key);
  int test2 = ires == -1;


  HS_add_element(&hs, &key, &val);
  key = 'B';
  val = '2';

  HS_add_element(&hs, &key, &val);
  ires = HS_remove_element(&hs, &key); // key == 'B'  
  int test3 = ires == 0 && hs._count == 1;
  key = 'A';;
  ires = HS_remove_element(&hs, &key);
  int test4 = ires == 0 && hs._count == 0 && hs._head == NULL;

  HS_destroy_hashmap(&hs);
  return test1 && test2 && test3 && test4;
}

int test_HSGetElement() {
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key;
  uint64_t val;
  void *ires;

  ires = HS_get_element(NULL, NULL);
  int test1 = ires == NULL;

  key = 'A';
  val = 1;
  // key not added yet
  ires = HS_get_element(&hs, &key);
  int test2 = ires == 0;
  
  HS_add_element(&hs, &key, &val);
  ires = HS_get_element(&hs, &key);
  int test3 = *(uint64_t *)ires == val;

  key = 'B';
  val = 2;
  HS_add_element(&hs, &key, &val);
  ires = HS_get_element(&hs, &key);
  int test4 = *(uint64_t *)ires == val;

  HS_destroy_hashmap(&hs);
  return test1 && test2 && test3 && test4;
}

void sum(void *key, void *val, void *payload) {
  (*(uint64_t*)payload) += (*(uint64_t*)val);
}

int test_HSTraverse() {
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key = 'A';
  uint64_t val = 1;
  uint64_t payload = 0;
  
  HS_traverse_hashmap_payload(&hs, sum, &payload);
  int test1 = payload == 0;
  
  HS_add_element(&hs, &key, &val);
  key = 'B'; val = 2;
  HS_add_element(&hs, &key, &val);
  HS_traverse_hashmap_payload(&hs, sum, &payload);
    
  int test2 = payload == 3;

  HS_destroy_hashmap(&hs);
  return test1 && test2;
}

int test_HSReserve() {
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key = 'A';
  uint64_t val = 1;
  HS_add_element(&hs, &key, &val);
  key = 'B'; val = 2;
  HS_add_element(&hs, &key, &val);

  HS_reserve(&hs, 100);
  key = 'A';
  int test1 = *(uint64_t *)HS_get_element(&hs, &key) == 1;
  key = 'B';
  int test2 = *(uint64_t *)HS_get_element(&hs, &key) == 2;
  
  HS_destroy_hashmap(&hs);
  return test1 && test2;
}

void benchmark() {
  printf("\n###### BENCHMARKING ######\n"
         "create_hashmap   : %s\n"
         "destroy_hashmap  : %s\n"
         "add_element      : %s\n"
         "remove_element   : %s\n"
         "get_element      : %s\n"
         "traverse_hashmap : %s\n"
         "reserve          : %s\n"
	 "##########################",
         (test_HSCreateHashmap() ? "SUCCESS" : "FAILURE"),
         (test_HSDestroyHashmap() ? "SUCCESS" : "FAILURE"),
         (test_HSAddElement() ? "SUCCESS" : "FAILURE"),
         (test_HSRemoveElement() ? "SUCCESS" : "FAILURE"),
         (test_HSGetElement() ? "SUCCESS" : "FAILURE"),
         (test_HSTraverse() ? "SUCCESS" : "FAILURE"),
         (test_HSReserve() ? "SUCCESS" : "FAILURE"));
}
