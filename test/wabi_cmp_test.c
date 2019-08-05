#define wabi_cmp_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_env.h"
#include "../src/wabi_cmp.h"

void
test_wabi_binary_compare_different_string_same_length(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "zfoo");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "afoo");
  ASSERT(wabi_cmp_raw(foo, bar) == 25);
}

void
test_wabi_binary_compare_different_string_same_prefix(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "fooz");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "fooa");
  ASSERT(wabi_cmp_raw(foo, bar) == 25);
}

void
test_wabi_binary_compare_same_string(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foos");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "foos");
  ASSERT(wabi_cmp_raw(foo, bar) == 0);
}

void
test_wabi_binary_compare_same_prefix_different_length(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foos");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "foo");

  ASSERT(wabi_cmp_raw(foo, bar) == 1);
}

void
test_wabi_binary_compare_left_is_node(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foo");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "bar");
  wabi_val foobar0 = wabi_binary_concat(vm, foo, bar);
  wabi_val foobar1 = wabi_binary_new_from_cstring(vm, "foobar");
  ASSERT(wabi_cmp_raw(foobar0, foobar1) == 0);
}

void
test_wabi_binary_compare_right_is_node(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foo");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "bar");
  wabi_val foobar0 = wabi_binary_concat(vm, foo, bar);
  wabi_val foobar1 = wabi_binary_new_from_cstring(vm, "foobar");
  ASSERT(wabi_cmp_raw(foobar1, foobar0) == 0);
}

void
test_wabi_binary_compare_node_various_overlappings(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foo");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "bar");
  wabi_val foob = wabi_binary_new_from_cstring(vm, "foob");
  wabi_val ar = wabi_binary_new_from_cstring(vm, "ar");
  wabi_val foobar0 = wabi_binary_concat(vm, foo, bar);
  wabi_val foobar1 = wabi_binary_concat(vm, foob, ar);
  wabi_val pos, len = wabi_smallint(vm, 2);

  for(long int j = 1; j < 5; j++) {
    pos = wabi_smallint(vm, j);
    ASSERT(wabi_cmp_raw(wabi_binary_sub(vm, foobar1, pos, len),
                        wabi_binary_sub(vm, foobar0, pos, len)) == 0);
  }
}

void
test_wabi_smallint_compare(wabi_vm vm)
{
  wabi_val l = wabi_smallint(vm, 626);
  wabi_val r = wabi_smallint(vm, 627);
  ASSERT(wabi_cmp_raw(l, r) < 0);
}


void test_wabi_env_compare(wabi_vm vm)
{

  wabi_env e0 = wabi_env_empty(vm);
  wabi_symbol k0 = wabi_binary_new_from_cstring(vm, "a");
  wabi_val v0 = wabi_smallint(vm, 10U);
  wabi_env_def(vm, e0, k0, v0);

  wabi_env e1 = wabi_env_empty(vm);
  wabi_symbol k1 = wabi_binary_new_from_cstring(vm, "a");
  wabi_val v1 = wabi_smallint(vm, 10U);
  wabi_env_def(vm, e1, k1, v1);


  ASSERT(wabi_cmp_raw((wabi_val) e0, (wabi_val) e1) == 0);
}


void test_wabi_env_extended(wabi_vm vm)
{

  wabi_env e = wabi_env_empty(vm);
  wabi_env e0 = wabi_env_extend(vm, e);
  wabi_symbol k0 = wabi_binary_new_from_cstring(vm, "a");
  wabi_val v0 = wabi_smallint(vm, 10U);
  wabi_env_def(vm, e0, k0, v0);

  wabi_env e1 = wabi_env_empty(vm);
  wabi_symbol k1 = wabi_binary_new_from_cstring(vm, "a");
  wabi_val v1 = wabi_smallint(vm, 10U);
  wabi_env_def(vm, e1, k1, v1);


  ASSERT(wabi_cmp_raw((wabi_val) e0, (wabi_val) e1) != 0);
  ASSERT(wabi_cmp_raw((wabi_val) e1, (wabi_val) e0) != 0);
}

void
wabi_cmp_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 10MB

  test_wabi_binary_compare_different_string_same_length(vm);
  test_wabi_binary_compare_different_string_same_prefix(vm);
  test_wabi_binary_compare_same_string(vm);
  test_wabi_binary_compare_same_prefix_different_length(vm);
  test_wabi_binary_compare_left_is_node(vm);
  test_wabi_binary_compare_right_is_node(vm);
  test_wabi_binary_compare_node_various_overlappings(vm);
  test_wabi_smallint_compare(vm);
  test_wabi_env_compare(vm);
  wabi_vm_free(vm);
  free(vm);
}
