#define wabi_map_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_mem.h"
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
wabi_map_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 100 * 1024 * 1024); // 100MB

  wabi_map_test_assoc_empty(vm);
  wabi_map_test_assoc_one(vm);
  wabi_map_test_iter(vm);
  /// wabi_map_test_order_do_not_affect_result(vm);

  wabi_mem_free(vm);
  free(vm);
}
