#define wabi_mem_test_c

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
wabi_mem_test_gc_smallint(wabi_vm vm) {
  wabi_val s, s0;
  s0 = wabi_smallint(vm, 100);
  vm->mem_root = s0;
  wabi_mem_collect(vm);

  s = wabi_smallint(vm, 100);
  ASSERT(wabi_eq_raw(vm->mem_root, s));
}


void
wabi_mem_test_gc_binary(wabi_vm vm) {
  wabi_val s, s0;
  s0 = wabi_binary_new_from_cstring(vm, "foo bar");
  vm->mem_root = s0;
  wabi_mem_collect(vm);

  s = wabi_binary_new_from_cstring(vm, "foo bar");
  ASSERT(wabi_eq_raw(vm->mem_root, s));
}


void
wabi_mem_test_gc_binary_sub(wabi_vm vm) {
  wabi_val s, s0;
  s0 = wabi_binary_new_from_cstring(vm, "foo bar");
  s0 = wabi_binary_sub(vm, s0, wabi_smallint(vm, 4), wabi_smallint(vm, 3));
  vm->mem_root = s0;
  wabi_mem_collect(vm);

  s = wabi_binary_new_from_cstring(vm, "bar");
  ASSERT(wabi_eq_raw(vm->mem_root, s));
}

void
wabi_mem_test_gc_binary_conc(wabi_vm vm) {
  wabi_val s, s0, s1;
  s = wabi_binary_new_from_cstring(vm, "foobar");
  s0 = wabi_binary_sub(vm, s, wabi_smallint(vm, 3), wabi_smallint(vm, 3));
  s1 = wabi_binary_sub(vm, s, wabi_smallint(vm, 0), wabi_smallint(vm, 3));
  s = wabi_binary_concat(vm, s0, s1);
  ASSERT(wabi_val_tag(s) == WABI_TAG_BIN_NODE);
  vm->mem_root = s;
  wabi_mem_collect(vm);

  s = wabi_binary_new_from_cstring(vm, "barfoo");
  ASSERT(wabi_eq_raw(vm->mem_root, s));
  ASSERT(wabi_val_tag(s) == WABI_TAG_BIN_LEAF);
}


void
wabi_mem_test_gc_map(wabi_vm vm) {
  wabi_val m;

  m = wabi_map_assoc(vm, wabi_map_empty(vm), wabi_smallint(vm, 1), wabi_binary_new_from_cstring(vm, "foo"));
  m = wabi_map_assoc(vm, m, wabi_smallint(vm, 2), wabi_binary_new_from_cstring(vm, "bar"));
  vm->mem_root = m;
  wabi_mem_collect(vm);
  m = wabi_map_assoc(vm, wabi_map_empty(vm), wabi_smallint(vm, 1), wabi_binary_new_from_cstring(vm, "foo"));
  m = wabi_map_assoc(vm, m, wabi_smallint(vm, 2), wabi_binary_new_from_cstring(vm, "bar"));
  // printf("%lx %lx\n", *(vm->mem_root), *(vm->mem_root + 1));
  ASSERT(wabi_eq_raw(vm->mem_root, m));
}


void
wabi_mem_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 1024 * 1024); // 64 MB
  // wabi_mem_test_gc_smallint(vm);
  // wabi_mem_test_gc_binary(vm);
  // wabi_mem_test_gc_binary_sub(vm);
  // wabi_mem_test_gc_binary_conc(vm);
  wabi_mem_test_gc_map(vm);

  wabi_mem_free(vm);
  free(vm);
}
