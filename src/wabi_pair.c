#define wabi_pair_c

#include "wabi_pair.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"
#include "wabi_env.h"
#include "wabi_error.h"
#include "wabi_value.h"
#include "wabi_vm.h"


static void wabi_pair_pair_car(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_car((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  if (wabi_is_nil(pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}

static void wabi_pair_pair_cdr(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_cdr((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  if (wabi_is_nil(pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}

static void wabi_pair_pair_cons(const wabi_vm vm) {
  wabi_val ctrl, a, d, res;

  ctrl = vm->ctrl;
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_val)wabi_cons(vm, a, d);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}

static void wabi_pair_pair_p(const wabi_vm vm) {
  wabi_builtin_predicate(vm, &wabi_is_pair);
}

void wabi_pair_builtins(const wabi_vm vm, const wabi_env env) {
  WABI_DEFN(vm, env, "car", "car", wabi_pair_pair_car);
  if (vm->ert)
    return;
  WABI_DEFN(vm, env, "cdr", "cdr", wabi_pair_pair_cdr);
  if (vm->ert)
    return;
  WABI_DEFN(vm, env, "cons", "cons", wabi_pair_pair_cons);
  if (vm->ert)
    return;
  WABI_DEFN(vm, env, "pair?", "pair?", wabi_pair_pair_p);
}
