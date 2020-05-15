#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


static inline wabi_error_type
wabi_pair_car_bt(wabi_vm vm, wabi_val pair)
{
  if(WABI_IS(wabi_tag_pair, pair)) {
    vm->ctrl = (wabi_val) wabi_car((wabi_pair) pair);
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  if(*pair == wabi_val_nil) {
    vm->ctrl = pair;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  return wabi_error_type_mismatch;
}


static inline wabi_error_type
wabi_pair_cdr_bt(wabi_vm vm, wabi_val pair)
{
  if(WABI_IS(wabi_tag_pair, pair)) {
    vm->ctrl = (wabi_val) wabi_cdr((wabi_pair) pair);
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  if(*pair == wabi_val_nil) {
    vm->ctrl = pair;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  return wabi_error_type_mismatch;
}


static inline wabi_error_type
wabi_pair_cons_bt(wabi_vm vm, wabi_val a, wabi_val d)
{
  wabi_val res;
  res = (wabi_val) wabi_cons(vm, a, d);
  if(res) {
    vm->ctrl = res;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_pair_p_bt(wabi_vm vm, wabi_val pair)
{
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = WABI_IS(wabi_tag_pair, pair) ? wabi_val_true : wabi_val_false;
    vm->ctrl = res;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_pair_builtin_car, wabi_pair_car_bt);
WABI_BUILTIN_WRAP1(wabi_pair_builtin_cdr, wabi_pair_cdr_bt);
WABI_BUILTIN_WRAP2(wabi_pair_builtin_cons, wabi_pair_cons_bt);
WABI_BUILTIN_WRAP1(wabi_pair_builtin_pair_p, wabi_pair_p_bt);

wabi_error_type
wabi_pair_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "car", "car", wabi_pair_builtin_car);
  if(res) return res;
  res = WABI_DEFN(vm, env, "cdr", "cdr", wabi_pair_builtin_cdr);
  if(res) return res;
  res = WABI_DEFN(vm, env, "cons", "cons", wabi_pair_builtin_cons);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pair?", "pair?", wabi_pair_builtin_pair_p);
  return res;
}
