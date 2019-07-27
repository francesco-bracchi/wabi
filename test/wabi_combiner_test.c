#define wabi_combiner_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_combiner.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_store.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_env.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_pr.h"

/**
 * TBD tests
 * 1. wrap
 * 2. wrap.wrap.wrap ... idempotent
 * 3. wrap . unwrap
 * 4. unwrap . wrap
 * 5. unwrap . wrap . unwrap ... idempotent
 * 6. create builtins
 * 7. create derived
 * 8. predicate operative/applicative
 */

// todo nil (and ignore) as special symbol

void
wabi_test_combiner_wrap(wabi_vm vm)
{
  wabi_val env = (wabi_val) wabi_env_empty(vm);
  wabi_val se = (wabi_val) wabi_binary_new_from_cstring(vm, "e");
  wabi_val e = wabi_intern(vm, se);
  wabi_val sa = (wabi_val) wabi_binary_new_from_cstring(vm, "a");
  wabi_val a = wabi_intern(vm, sa);
  wabi_val fs = wabi_cons(vm, a, wabi_nil(vm));
  wabi_val splus = (wabi_val) wabi_binary_new_from_cstring(vm, "+");
  wabi_val plus = wabi_intern(vm, splus);
  wabi_val body = wabi_cons(vm, plus,
                            wabi_cons(vm, wabi_smallint(vm, 1),
                                      wabi_cons(vm, a,
                                                wabi_nil(vm))));

  // wabi_pr(body);
  wabi_val oper = (wabi_val) wabi_combiner_new(vm, (wabi_env) env, (wabi_symbol) e, fs, body);
  wabi_val app = (wabi_val) wabi_combiner_wrap(vm, oper);

  ASSERT(WABI_COMBINER_IS_OPERATIVE(oper) != 0U);
  ASSERT(WABI_COMBINER_IS_APPLICATIVE(app) != 0U);
}

void
wabi_test_combiner_unwrap(wabi_vm vm)
{
  wabi_val env = (wabi_val) wabi_env_empty(vm);
  wabi_val se = (wabi_val) wabi_binary_new_from_cstring(vm, "e");
  wabi_val e = wabi_intern(vm, se);
  wabi_val sa = (wabi_val) wabi_binary_new_from_cstring(vm, "a");
  wabi_val a = wabi_intern(vm, sa);
  wabi_val fs = wabi_cons(vm, a, wabi_nil(vm));
  wabi_val splus = (wabi_val) wabi_binary_new_from_cstring(vm, "+");
  wabi_val plus = wabi_intern(vm, splus);
  wabi_val body = wabi_cons(vm, plus,
                            wabi_cons(vm, wabi_smallint(vm, 1),
                                      wabi_cons(vm, a,
                                                wabi_nil(vm))));

  // wabi_pr(body);
  wabi_val oper = (wabi_val) wabi_combiner_new(vm, (wabi_env) env, (wabi_symbol) e, fs, body);
  wabi_val app = (wabi_val) wabi_combiner_wrap(vm, oper);
  wabi_val oper1 = (wabi_val) wabi_combiner_unwrap(vm, app);
  ASSERT(wabi_cmp_raw(oper, oper1) == 0);
}


static wabi_val
wabi_test_inc(wabi_vm vm, wabi_val args)
{
  if(wabi_val_is_pair(args)) {
    wabi_val first = wabi_car(vm, args);
    if(wabi_val_is_smallint(first)) {
      return (wabi_val) wabi_smallint(vm, 1 + (*first & WABI_VALUE_MASK));
    }
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}

void
wabi_test_combiner_create_builtin(wabi_vm vm)
{
  wabi_val oper = (wabi_val) wabi_combiner_builtin_new(vm, wabi_test_inc);
  wabi_val app = (wabi_val) wabi_combiner_wrap(vm, oper);

  ASSERT(WABI_COMBINER_IS_APPLICATIVE(app) != 0);
  ASSERT(WABI_COMBINER_IS_BUILTIN(app) != 0);
}


void
wabi_combiner_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 10MB

  wabi_test_combiner_wrap(vm);
  wabi_test_combiner_unwrap(vm);
  wabi_test_combiner_create_builtin(vm);

  wabi_vm_free(vm);
  free(vm);
}
