#define wabi_vm_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_env.h"
#include "../src/wabi_pr.h"

void
test_wabi_vm_bind_ignore(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_val i = wabi_ignore(vm);
  wabi_val v = wabi_smallint(vm, 10);
  wabi_vm_bind(vm, (wabi_val) e, i, v);
  ASSERT(wabi_cmp_raw(e, wabi_env_empty(vm)) == 0);
}


void
test_wabi_vm_bind_var(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_val b = wabi_binary_new_from_cstring(vm, "a");
  wabi_val s = wabi_intern(vm, b);
  wabi_val v0 = wabi_smallint(vm, 10);
  wabi_vm_bind(vm, (wabi_val) e, s, v0);
  wabi_val v = wabi_env_lookup(e, (wabi_symbol) s);
  ASSERT(v == v0);
}


void
test_wabi_vm_bind_pair(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_val ab = wabi_binary_new_from_cstring(vm, "a");
  wabi_val sa = wabi_intern(vm, ab);
  wabi_val asb = wabi_binary_new_from_cstring(vm, "as");
  wabi_val sas = wabi_intern(vm, asb);
  wabi_val pat = wabi_cons(vm, sa, sas);

  wabi_val v0 = wabi_smallint(vm, 10);
  wabi_val v1 = wabi_smallint(vm, 20);
  wabi_val nil = wabi_nil(vm);
  wabi_val val = wabi_cons(vm, v0, wabi_cons(vm, v1, nil));
  wabi_vm_bind(vm, (wabi_val) e, pat, val);
  wabi_val v = wabi_env_lookup(e, (wabi_symbol) sas);
  ASSERT(v != NULL);
  if(v)
    ASSERT(wabi_cmp_raw(v, wabi_cons(vm, v1, nil)) == 0);

  v = wabi_env_lookup(e, (wabi_symbol) sa);
  ASSERT(v != NULL);
  if(v)
    ASSERT(wabi_cmp_raw(v, v0) == 0);
}

void
test_wabi_vm_arity_error(wabi_vm vm)
{
  wabi_env e = wabi_env_empty(vm);
  wabi_val ab = wabi_binary_new_from_cstring(vm, "a");
  wabi_val sa = wabi_intern(vm, ab);
  wabi_val pat = wabi_cons(vm, sa, wabi_nil(vm));
  wabi_val val = wabi_nil(vm);
  wabi_vm_bind(vm, (wabi_val) e, pat, val);
  ASSERT(vm->errno != 0);
}

void
wabi_vm_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 10MB

  test_wabi_vm_bind_var(vm);
  test_wabi_vm_bind_pair(vm);
  test_wabi_vm_bind_ignore(vm);
  test_wabi_vm_arity_error(vm);

  wabi_vm_free(vm);
  free(vm);
}
