#define wabi_map_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_map.h"
#include "../src/wabi_pr.h"


void
wabi_map_test_assoc_empty(wabi_vm vm)
{
  wabi_val m = wabi_map_empty(vm);
  wabi_val k = wabi_smallint(vm, 1);
  wabi_val v = wabi_binary_new_from_cstring(vm, "one");

  m = wabi_map_assoc(vm, m, k, v);

  ASSERT(wabi_map_length_raw((wabi_map) m) == 1);
}


void
wabi_map_test_assoc_many(wabi_vm vm)
{
  char str[100];
  wabi_val m, k, v;
  int j, size;

  size = 3 * 1000;
  m = wabi_map_empty(vm);
  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_smallint(vm, j);

    if (vm->errno) {
      printf("x");
      vm->store.root = m;
      wabi_vm_collect(vm);
      m = vm->store.root;
      j--;
      continue;
    }

    v = wabi_binary_new_from_cstring(vm, str);

    if (vm->errno) {
      printf("y");
      vm->store.root = m;
      wabi_vm_collect(vm);
      m = vm->store.root;
      j--;
      continue;
    }
    wabi_val m0 = m;
    m = wabi_map_assoc(vm, m, k, v);

    if (vm->errno) {
      printf("z");
      vm->store.root = m0;
      wabi_vm_collect(vm);
      m = vm->store.root;
      j--;
      continue;
    }
  }
  // wabi_map_tree((wabi_map) m);

  /* for(j = 0; j < 64; j+=17) { */
  /*   sprintf(str, "%iK", j); */
  /*   k = wabi_smallint(vm, j); */
  /*   v = wabi_binary_new_from_cstring(vm, str); */
  /*   ASSERT(wabi_eq_raw(wabi_map_get(vm, m, k), v)); */
  /* } */
}


void
wabi_map_test_reassoc_many(wabi_vm vm)
{
  int size = 100;
  char str[100];
  wabi_val m, k, v;
  int j;

  m = wabi_map_empty(vm);
  for(j = 0; j < size ;j++) {
    sprintf(str, "%iK", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  m = wabi_map_empty(vm);
  for(j = 0; j < size ;j++) {
    sprintf(str, "%iK", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  for(j = 0; j < size ;j++) {
    sprintf(str, "%iH", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  for(j = 0; j < size ;j+= 19) {
    sprintf(str, "%iH", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    ASSERT(wabi_eq_raw(wabi_map_get(vm, m, k), v));
  }
}


void
wabi_map_test_assoc_one(wabi_vm vm)
{
  wabi_val m = wabi_map_empty(vm);
  wabi_val k = wabi_smallint(vm, 1);
  wabi_val v = wabi_binary_new_from_cstring(vm, "one");
  m = wabi_map_assoc(vm, m, k, v);
  k = wabi_smallint(vm, 2);
  v = wabi_binary_new_from_cstring(vm, "two");

  m = wabi_map_assoc(vm, m, k, v);

  ASSERT(wabi_map_length_raw((wabi_map) m) == 2);
}


void
wabi_map_test_iter(wabi_vm vm)
{
  wabi_val m, k, v;
  wabi_map_iter_t iter;
  wabi_map_entry entry;

  m  = wabi_map_empty(vm);
  wabi_map_iterator_init(&iter, (wabi_map) m);

  ASSERT(wabi_map_iterator_current(&iter) == NULL);

  k = wabi_smallint(vm, 1);
  v = wabi_binary_new_from_cstring(vm, "one");
  m = wabi_map_assoc(vm, m, k, v);
  wabi_map_iterator_init(&iter, (wabi_map) m);

  entry = wabi_map_iterator_current(&iter);

  ASSERT(entry != NULL);
  ASSERT(wabi_eq_raw(WABI_MAP_ENTRY_KEY(entry), k));

  wabi_map_iterator_next(&iter);
  entry = wabi_map_iterator_current(&iter);

  ASSERT(entry == NULL);
}

void
wabi_map_test_iter_on_hashes(wabi_vm vm)
{
  char str[100];
  wabi_val m, k, v;
  wabi_map_iter_t iter;
  wabi_map_entry entry;
  int len = 360;

  m  = wabi_map_empty(vm);
  for(int j = 0; j < len; j++) {
    sprintf(str, "N%i", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  wabi_map_iterator_init(&iter, (wabi_map) m);
  int cnt = 0;
  while((entry = wabi_map_iterator_current(&iter))) {
    cnt++;
    wabi_map_iterator_next(&iter);
  }
  ASSERT(cnt == len);
}


void
wabi_map_test_order_do_not_affect_result(wabi_vm vm)
{
  wabi_val m, k, v, m0;

  m  = wabi_map_empty(vm);
  m0 = m;
  k = wabi_smallint(vm, 1);
  v = wabi_binary_new_from_cstring(vm, "one");
  m = wabi_map_assoc(vm, m, k, v);
  k = wabi_smallint(vm, 2);
  v = wabi_binary_new_from_cstring(vm, "two");
  m = wabi_map_assoc(vm, m, k, v);

  k = wabi_smallint(vm, 2);
  v = wabi_binary_new_from_cstring(vm, "two");
  m0 = wabi_map_assoc(vm, m0, k, v);
  k = wabi_smallint(vm, 1);
  v = wabi_binary_new_from_cstring(vm, "one");
  m0 = wabi_map_assoc(vm, m0, k, v);

  ASSERT(wabi_eq_raw(m, m0));
}


void
wabi_map_test_dissoc_one(wabi_vm vm)
{

  wabi_val m, k, v;
  m  = wabi_map_empty(vm);
  k = wabi_smallint(vm, 1);
  v = wabi_binary_new_from_cstring(vm, "one");
  m = wabi_map_assoc(vm, m, k, v);
  m = wabi_map_dissoc(vm, m, k);

  ASSERT(wabi_eq_raw(m, wabi_map_empty(vm)));
}


void
wabi_map_test_dissoc_length_lt_limit(wabi_vm vm)
{
  char str[100];
  wabi_val m, k, v;
  m  = wabi_map_empty(vm);
  for(int j = 0; j < WABI_MAP_ARRAY_LIMIT; j++) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  k = wabi_smallint(vm, 7);
  m = wabi_map_dissoc(vm, m, k);

  ASSERT(wabi_map_length_raw((wabi_map) m) == WABI_MAP_ARRAY_LIMIT - 1);
}


void
wabi_map_test_dissoc_length_gt_limit(wabi_vm vm)
{
  char str[100];
  wabi_val m, k, v;
  int size = 10000;
  m  = wabi_map_empty(vm);
  for(int j = 0; j < size; j++) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  k = wabi_smallint(vm, 7);
  m = wabi_map_dissoc(vm, m, k);
  ASSERT(wabi_map_length_raw((wabi_map) m) == size - 1);
}


void
wabi_map_test_dissoc_demote(wabi_vm vm)
{
  char str[100];
  wabi_val m0, m, k, v;
  int size = WABI_MAP_ARRAY_LIMIT + 1;

  m  = wabi_map_empty(vm);
  m0 = m;
  for(int j = 0; j < size - 1; j++) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m0 = wabi_map_assoc(vm, m0, k, v);
  }

  for(int j = 0; j < size; j++) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  k = wabi_smallint(vm, WABI_MAP_ARRAY_LIMIT);
  m = wabi_map_dissoc(vm, m, k);

  ASSERT(wabi_cmp_raw(m, m0) == 0);
}

void
wabi_map_test_hash(wabi_vm vm)
{
  char str[100];
  wabi_val m0, m, k, v;
  int size = WABI_MAP_ARRAY_LIMIT + 10;

  m  = wabi_map_empty(vm);
  m0 = m;
  for(int j = 0; j < size; j++) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m0 = wabi_map_assoc(vm, m0, k, v);
  }

  for(int j = size - 1; j >= 0; j--) {
    sprintf(str, "%iN", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m = wabi_map_assoc(vm, m, k, v);
  }
  ASSERT(wabi_hash_raw(m) == wabi_hash_raw(m0));
}


void
wabi_map_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 64 * 1024 * 1024);

  wabi_map_test_assoc_empty(vm);
  wabi_map_test_assoc_one(vm);
  wabi_map_test_assoc_many(vm);
  wabi_map_test_iter(vm);
  wabi_map_test_iter_on_hashes(vm);
  wabi_map_test_order_do_not_affect_result(vm);
  wabi_map_test_reassoc_many(vm);
  wabi_map_test_dissoc_one(vm);
  wabi_map_test_dissoc_length_lt_limit(vm);
  wabi_map_test_dissoc_length_gt_limit(vm);
  wabi_map_test_dissoc_demote(vm);
  wabi_map_test_hash(vm);

  wabi_vm_free(vm);
  free(vm);
}
