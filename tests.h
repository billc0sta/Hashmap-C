#ifndef HSTESTS
#define HSTESTS
#include "hashmap.h"
#include <stdio.h>
#include <sys/time.h>

int test_HSCreateHashmap(double *el);
int test_HSDestroyHashmap(double *el);
int test_HSAddElement(double *el);
int test_HSRemoveElement(double *el);
int test_HSGetElement(double *el);
void sum(void *key, void *val, void *payload);
int test_HSTraverse(double *el);
int test_HSReserve(double *el);
void benchmark();

struct timeval start_timer() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t;
}

double get_elapsed(struct timeval *t) {
  double elapsed_time; 
  struct timeval t2;
  gettimeofday(&t2, NULL);
  
  elapsed_time = (t2.tv_sec - t->tv_sec) * 1000.0;      // sec to ms
  elapsed_time += (t2.tv_usec - t->tv_usec) / 1000.0;   // us to ms
  return elapsed_time;
}

int test_HSCreateHashmap(double *el) {
  struct timeval t = start_timer();
  
  Hashmap hs1 = HS_create_hashmap(0, 1);
  Hashmap hs2 = HS_create_hashmap(10, 10);
  int test1 = hs2._key_typesize == 10 && hs2._val_typesize == 10 &&
              hs2._reserved == BASE_SIZE;

  HS_destroy_hashmap(&hs2);
  *el = get_elapsed(&t);
  return test1;
}

int test_HSDestroyHashmap(double *el) {
  struct timeval t = start_timer(); 
  // should end program
  // HS_destroy_hashmap(NULL);
  Hashmap hs = HS_create_hashmap(1, 1);

  HS_destroy_hashmap(&hs);
  int test1 = hs._reserved == 0 && hs._key_typesize == 0 &&
              hs._val_typesize == 0 && hs._arr == NULL && hs._count == 0;

  *el = get_elapsed(&t);
  return test1;
}

int test_HSAddElement(double *el) {
  struct timeval t = start_timer();
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
  *el = get_elapsed(&t);
  return test1 && test2 && test3;
}

int test_HSRemoveElement(double *el) {
  struct timeval t = start_timer();
  Hashmap hs = HS_create_hashmap(sizeof(char), sizeof(uint64_t));
  char key;
  uint64_t val;
  int ires;
  key = 'A';
  val = 1;
  
  ires = HS_remove_element(NULL, NULL);
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
  *el = get_elapsed(&t); 
  return test1 && test2 && test3 && test4;
}

int test_HSGetElement(double *el) {
  struct timeval t = start_timer();
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
  *el = get_elapsed(&t);
  return test1 && test2 && test3 && test4;
}

void sum(void *key, void *val, void *payload) {
  (*(uint64_t*)payload) += (*(uint64_t*)val);
}

int test_HSTraverse(double *el) {
  struct timeval t = start_timer();
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
  *el = get_elapsed(&t);
  return test1 && test2;
}

int test_HSReserve(double *el) {
  struct timeval t = start_timer();
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
  *el = get_elapsed(&t);
  return test1 && test2;
}

void benchmark() {
  struct timeval t = start_timer();
  double elapseds;

  printf(
	 "\n<----------- BENCHMARKING ----------->\n"
         "create_hashmap   : %s | %fms\n"
         "destroy_hashmap  : %s | %fms\n"
         "add_element      : %s | %fms\n"
         "remove_element   : %s | %fms\n"
         "get_element      : %s | %fms\n"
         "traverse_hashmap : %s | %fms\n"
         "reserve          : %s | %fms\n"
         "<------------------------------------>",
         (test_HSCreateHashmap(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSDestroyHashmap(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSAddElement(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSRemoveElement(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSGetElement(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSTraverse(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds,
         (test_HSReserve(&elapseds) ? "SUCCESS" : "FAILURE"), elapseds);
  double elapsed_time = get_elapsed(&t);
  printf("\nTOTAL TIME ELAPSED: %fms\n", elapsed_time);
}

#endif 
