#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


static inline wabi_val
wabi_pair_car_bt(wabi_vm vm, wabi_env env, wabi_val pair)
{
  if(WABI_IS(wabi_tag_pair, pair)) {
    return (wabi_val) wabi_car((wabi_pair) pair);
  }
  if(*pair == wabi_val_nil) {
    return pair;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


static inline wabi_val
wabi_pair_cdr_bt(wabi_vm vm, wabi_env env, wabi_val pair)
{
  if(WABI_IS(wabi_tag_pair, pair)) {
    return (wabi_val) wabi_cdr((wabi_pair) pair);
  }
  if(*pair == wabi_val_nil) {
    return pair;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


static inline wabi_val
wabi_pair_cons_bt(wabi_vm vm, wabi_env env, wabi_val a, wabi_val d)
{
  if(wabi_vm_has_rooms(vm, WABI_PAIR_SIZE)) {
    return (wabi_val) wabi_cons(vm, a, d);
  }
  vm->errno = wabi_error_nomem;
  return vm->control;
}


static inline wabi_val
wabi_pair_p_bt(wabi_vm vm, wabi_env env, wabi_val pair)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = (wabi_val) wabi_vm_alloc(vm, 1);
    *res = WABI_IS(wabi_tag_pair, pair) ? wabi_val_true : wabi_val_false;
    return res;
  }
  vm->errno = wabi_error_nomem;
  return vm->control;
}


WABI_BUILTIN_WRAP1(wabi_pair_builtin_car, wabi_pair_car_bt);
WABI_BUILTIN_WRAP1(wabi_pair_builtin_cdr, wabi_pair_cdr_bt);
WABI_BUILTIN_WRAP2(wabi_pair_builtin_cons, wabi_pair_cons_bt);
WABI_BUILTIN_WRAP1(wabi_pair_builtin_pair_p, wabi_pair_p_bt);

void
wabi_pair_builtins(wabi_vm vm, wabi_env env)
{
  WABI_DEFN(vm, env, "car", "wabi:car", wabi_pair_builtin_car);
  WABI_DEFN(vm, env, "cdr", "wabi:cdr", wabi_pair_builtin_cdr);
  WABI_DEFN(vm, env, "cons", "wabi:cons", wabi_pair_builtin_cons);
  WABI_DEFN(vm, env, "pair?", "wabi:pair?", wabi_pair_builtin_pair_p);
}
