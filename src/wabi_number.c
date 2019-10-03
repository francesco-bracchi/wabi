#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_number.h"
#include "wabi_builtin.h"
#include "wabi_error.h"

wabi_fixnum
wabi_fixnum_new(wabi_vm vm,
                int64_t val)
{
  if(wabi_vm_has_rooms(vm, 1)) {
    wabi_val res = wabi_vm_alloc(vm, 1);
    *res = val & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    return res;
  }
  return NULL;
}


static inline wabi_val
wabi_number_sum_bt(wabi_vm vm, wabi_env env, wabi_val x, wabi_val y)
{
  wabi_val res;
  if(WABI_IS(wabi_tag_fixnum, x) && WABI_IS(wabi_tag_fixnum, y)) {
    // do bound checks
    if(wabi_vm_has_rooms(vm, 1)) {
      *res = WABI_CAST_INT64(x) + WABI_CAST_INT64(y);
      WABI_SET_TAG(res, wabi_tag_fixnum);
      return  res;
    }
    vm->errno = wabi_error_nomem;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


static inline wabi_val
wabi_number_diff_bt(wabi_vm vm, wabi_env env, wabi_val x, wabi_val y)
{
  wabi_val res;
  if(WABI_IS(wabi_tag_fixnum, x) && WABI_IS(wabi_tag_fixnum, y)) {
    // do bound checks
    if(wabi_vm_has_rooms(vm, 1)) {
      *res = WABI_CAST_INT64(x) - WABI_CAST_INT64(y);
      WABI_SET_TAG(res, wabi_tag_fixnum);
      return  res;
    }
    vm->errno = wabi_error_nomem;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


static inline wabi_val
wabi_number_div_bt(wabi_vm vm, wabi_env env, wabi_val x, wabi_val y)
{
  wabi_val res;
  if(WABI_IS(wabi_tag_fixnum, x) && WABI_IS(wabi_tag_fixnum, y)) {
    // do bound checks
    if(WABI_CAST_INT64(x)) {
      if(wabi_vm_has_rooms(vm, 1)) {
        *res = WABI_CAST_INT64(x) / WABI_CAST_INT64(y);
        WABI_SET_TAG(res, wabi_tag_fixnum);
        return  res;
      }
      vm->errno = wabi_error_nomem;
      return vm->control;
    }
    vm->errno = wabi_error_division_by_zero;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}

static inline wabi_val
wabi_number_mul_bt(wabi_vm vm, wabi_env env, wabi_val x, wabi_val y)
{
  wabi_val res;
  if(WABI_IS(wabi_tag_fixnum, x) && WABI_IS(wabi_tag_fixnum, y)) {
    // do bound checks
    if(wabi_vm_has_rooms(vm, 1)) {
      *res = WABI_CAST_INT64(x) * WABI_CAST_INT64(y);
      WABI_SET_TAG(res, wabi_tag_fixnum);
      return  res;
    }
    vm->errno = wabi_error_nomem;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


WABI_BUILTIN_WRAP1_PLUS(wabi_number_sum_builtin, wabi_number_sum_bt)
WABI_BUILTIN_WRAP1_PLUS(wabi_number_diff_builtin, wabi_number_diff_bt)
WABI_BUILTIN_WRAP1_PLUS(wabi_number_mul_builtin, wabi_number_sum_bt)
WABI_BUILTIN_WRAP1_PLUS(wabi_number_div_builtin, wabi_number_div_bt)
