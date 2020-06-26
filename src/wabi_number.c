#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_constant.h"
#include "wabi_number.h"
#include "wabi_combiner.h"
#include "wabi_builtin.h"
#include "wabi_error.h"


wabi_fixnum
wabi_fixnum_new(const wabi_vm vm,
                const int64_t val)
{
  wabi_val res;

  if(val >= wabi_fixnum_max) {
    vm->ert = wabi_error_out_of_range;
    return NULL;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return NULL;

  *res = val & wabi_word_value_mask;
  WABI_SET_TAG(res, wabi_tag_fixnum);
  return res;
}


static void
wabi_number_builtin_sum(const wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 0L;
  ctrl = vm->ctrl;

  a = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac += WABI_CAST_INT64(a);
  }
  if(!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  *a = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->ctrl = a;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_number_builtin_mul(const wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 1L;
  ctrl = vm->ctrl;

  a = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac *= WABI_CAST_INT64(a);
  }
  if(! wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  *a = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  vm->ctrl = a;
}


static void
wabi_number_builtin_diff(const wabi_vm vm)
{
  long ac;
  wabi_val a, res, ctrl;

  ctrl = vm->ctrl;

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  ac = WABI_CAST_INT64(a);

  if(wabi_is_nil(ctrl)) {
    // unary op
    *res = (- ac) & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    vm->ctrl = res;
    return;
  }

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    ac -= WABI_CAST_INT64(a);
  }
  if(! wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
  }
  *res = ac & wabi_word_value_mask;
  WABI_SET_TAG(res, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  vm->ctrl = res;
}


static void
wabi_number_builtin_div(const wabi_vm vm)
{
  long x, ac;
  wabi_val a, ctrl, res;

  ctrl = vm->ctrl;

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  ac = WABI_CAST_INT64(a);

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    x = WABI_CAST_INT64(a);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(x == 0) {
      vm->ert = wabi_error_division_by_zero;
      return;
    }
    ac /= x;
  }
  if(! wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  *res = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  vm->ctrl = res;
}


void
wabi_number_builtins(const wabi_vm vm, const wabi_env env)
{
  WABI_DEFN(vm, env, "+", "+", wabi_number_builtin_sum);
  if(vm->ert) return;
  WABI_DEFN(vm, env, "*", "*", wabi_number_builtin_mul);
  if(vm->ert) return;
  WABI_DEFN(vm, env, "-", "-", wabi_number_builtin_diff);
  if(vm->ert) return;
  WABI_DEFN(vm, env, "/", "/", wabi_number_builtin_div);
}
