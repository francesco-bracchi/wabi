#define wabi_binary_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_env.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_cmp.h"
#include "../src/wabi_pr.h"


void
test_wabi_env_insert(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_symbol k0 = wabi_binary_new_from_cstring(vm, "a");
  wabi_symbol k1 = wabi_binary_new_from_cstring(vm, "b");
  wabi_val v = wabi_smallint(vm, 10U);
  wabi_env_assoc(vm, e, k0, v);

  wabi_val v0 = wabi_env_lookup(e, k0);
  wabi_val v1 = wabi_env_lookup(e, k1);

  ASSERT(wabi_cmp_raw(v, v0) == 0);
  ASSERT(v1 == NULL)
}


void
test_wabi_env_shadow(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_symbol k = wabi_binary_new_from_cstring(vm, "a");
  wabi_val v0 = wabi_smallint(vm, 10U);
  wabi_val v1 = wabi_smallint(vm, 20U);
  wabi_env_assoc(vm, e, k, v0);
  wabi_env e1 = wabi_env_extend(vm, e);
  wabi_env_assoc(vm, e1, k, v1);

  wabi_val vx0 = wabi_env_lookup(e, k);
  wabi_val vx1 = wabi_env_lookup(e1, k);

  ASSERT(wabi_cmp_raw(vx0, v0) == 0);
  ASSERT(wabi_cmp_raw(vx1, v1) == 0);
}


void
wabi_env_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 2MB

  test_wabi_env_insert(vm);
  test_wabi_env_shadow(vm);

  wabi_vm_free(vm);
  free(vm);
}
