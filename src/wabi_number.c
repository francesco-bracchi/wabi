#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_number.h"
#include "wabi_combiner.h"
#include "wabi_builtin.h"
#include "wabi_error.h"


wabi_fixnum
wabi_fixnum_new(wabi_vm vm,
                int64_t val)
{
  wabi_val res;
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = val & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    return res;
  }
  return NULL;
}


wabi_error_type
wabi_number_builtin_sum(wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 0L;
  ctrl = vm->ctrl;

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac += WABI_CAST_INT64(a);
  }
  if(*ctrl == wabi_val_nil) {
    a = wabi_vm_alloc(vm, 1);
    if(a) {
      *a = ac & wabi_word_value_mask;
      WABI_SET_TAG(a, wabi_tag_fixnum);
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      vm->ctrl = a;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_number_builtin_mul(wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 1L;
  ctrl = vm->ctrl;

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac *= WABI_CAST_INT64(a);
  }
  if(*ctrl == wabi_val_nil) {
    a = wabi_vm_alloc(vm, 1);
    if(a) {
      *a = ac& wabi_word_value_mask;
      WABI_SET_TAG(a, wabi_tag_fixnum);
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      vm->ctrl = a;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_number_builtin_diff(wabi_vm vm)
{
  long ac;
  wabi_val a, res, ctrl;

  ctrl = vm->ctrl;

  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    return wabi_error_bindings;
  }
  res = wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  ac = WABI_CAST_INT64(a);

  if(*ctrl == wabi_val_nil) {
    *res = (- ac) & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    vm->ctrl = res;
    return wabi_error_none;
  }

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac -= WABI_CAST_INT64(a);
  }
  if(*ctrl == wabi_val_nil) {
    *res = ac & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    vm->ctrl = res;
    return wabi_error_none;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_number_builtin_div(wabi_vm vm)
{
  long x, ac;
  wabi_val a, ctrl, res;

  ctrl = vm->ctrl;

  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    return wabi_error_bindings;
  }
  res = wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  ac = WABI_CAST_INT64(a);

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    x = WABI_CAST_INT64(a);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(x == 0) {
      return wabi_error_division_by_zero;
    }
    ac /= x;
  }
  if(*ctrl != wabi_val_nil)
    return wabi_error_bindings;

  res = wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  *res = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  vm->ctrl = res;
  return wabi_error_none;
}


wabi_error_type
wabi_number_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "+", "+", wabi_number_builtin_sum);
  if(res) return res;
  res = WABI_DEFN(vm, env, "*", "*", wabi_number_builtin_mul);
  if(res) return res;
  res = WABI_DEFN(vm, env, "-", "-", wabi_number_builtin_diff);
  if(res) return res;
  res = WABI_DEFN(vm, env, "/", "/", wabi_number_builtin_div);
  return res;
}
