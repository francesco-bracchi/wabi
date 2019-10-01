#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"


wabi_pair
wabi_cons(wabi_vm vm, wabi_val car, wabi_val cdr)
{
  wabi_pair pair = (wabi_pair) wabi_vm_alloc(vm, WABI_PAIR_SIZE);
  if(pair) {
    pair->car = (wabi_word) car;
    pair->cdr = (wabi_word) cdr;
    WABI_SET_TAG(pair, wabi_tag_pair);
  }
  return pair;
}


void
wabi_pair_builtin_car(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, pair;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    pair = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      if(WABI_IS(wabi_tag_pair, pair)) {
        vm->control = (wabi_val) wabi_car((wabi_pair) pair);
        return;
      }
      if(*pair == wabi_val_nil) {
        vm->control = pair;
        return;
      }
      vm->errno = wabi_error_type_mismatch;
      return;
    }
  }
  vm->errno = wabi_error_bindings;
}


void
wabi_pair_builtin_cdr(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, pair;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    pair = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      if(WABI_IS(wabi_tag_pair, pair)) {
        vm->control = (wabi_val) wabi_cdr((wabi_pair) pair);
        return;
      }
      if(*pair == wabi_val_nil) {
        vm->control = pair;
        return;
      }
      vm->errno = wabi_error_type_mismatch;
      return;
    }
  }
  vm->errno = wabi_error_bindings;
}


void
wabi_pair_builtin_cons(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, car, cdr;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    car = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      cdr = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(*ctrl == wabi_val_nil) {
        ctrl = (wabi_val) wabi_cons(vm, car, cdr);
        if(ctrl) {
          vm->control = ctrl;
          return;
        }
        vm->errno = wabi_error_nomem;
      }
    }
  }
  vm->errno = wabi_error_bindings;
}


void
wabi_builtin_pair_p(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, val, res;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      res = (wabi_val) wabi_vm_alloc(vm, 1);
      if(res) {
        *res = WABI_IS(wabi_tag_pair, val) ? wabi_val_true : wabi_val_false;
        vm->control = res;
        return;
      }
      vm->errno = wabi_error_nomem;
    }
  }
  vm->errno = wabi_error_bindings;
}
