#define wabi_list_c

#include "wabi_list.h"
#include "wabi_number.h"
#include "wabi_builtin.h"
#include "wabi_atom.h"
#include "wabi_env.h"
#include "wabi_error.h"
#include "wabi_value.h"
#include "wabi_vm.h"


static void
wabi_list_car(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_car((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }

  if (wabi_atom_is_empty(vm, pair) || wabi_atom_is_nil(vm, pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}


static void
wabi_list_cdr(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_cdr((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  if (wabi_atom_is_empty(vm, pair) || wabi_atom_is_nil(vm, pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}


static void
wabi_list_len(const wabi_vm vm) {
  wabi_val ctrl, list;
  wabi_size l;
  wabi_fixnum r;

  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  list = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  l = wabi_list_length(vm, list);
  if(l < 0) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  r = wabi_fixnum_new(vm, l);
  if(vm->ert) return;
  vm->ctrl = (wabi_val) r;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}


static void wabi_list_cons(const wabi_vm vm) {
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
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_val)wabi_cons(vm, a, d);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}


static void wabi_list_pair_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_pair);
}


static void wabi_list_list_p(const wabi_vm vm)
{

  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_list(vm, val)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


void
wabi_list_builtins(const wabi_vm vm,
                   const wabi_env env)
{
  wabi_defn(vm,env, "cons", &wabi_list_cons);
  if(vm->ert) return;
  wabi_defn(vm,env, "car", &wabi_list_car);
  if(vm->ert) return;
  wabi_defn(vm,env, "cdr", &wabi_list_cdr);
  if(vm->ert) return;
  wabi_defn(vm,env, "pair?", &wabi_list_pair_p);
  if(vm->ert) return;
  wabi_defn(vm,env, "list?", &wabi_list_list_p);
  if(vm->ert) return;
  wabi_defn(vm,env, "len", &wabi_list_len);
  if(vm->ert) return;
}
