#define wabi_place_c

#include <stdio.h>

#include "wabi_place.h"
#include "wabi_error.h"
#include "wabi_env.h"
#include "wabi_list.h"
#include "wabi_atom.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"


static inline wabi_word
wabi_place_uid(const wabi_place place)
{
  static wabi_word wabi_place_cnt = 0;
  return ((wabi_word) place) ^ (++wabi_place_cnt);
}


wabi_place
wabi_place_new(const wabi_vm vm,
               const wabi_val init)
{
  wabi_place place = (wabi_place) wabi_vm_alloc(vm, WABI_PLACE_SIZE);
  if(vm->ert) return NULL;

  place->uid = wabi_place_uid(place);
  place->val = (wabi_word) init;
  WABI_SET_TAG(place, wabi_tag_place);
  return place;
}


static void
wabi_place_plc(const wabi_vm vm)
{
  wabi_val ctrl, init;
  wabi_place res;

  ctrl = vm->ctrl;
  init = vm->nil;

  if(wabi_is_pair(ctrl)) {
    init = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_place_new(vm, init);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_place_plc_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_place);
}

static void
wabi_place_plc_val(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(!wabi_is_place((wabi_val) plc)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = wabi_place_val(plc);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_place_plc_cas(const wabi_vm vm)
{
  wabi_val ctrl, res;
  wabi_word old, new;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_place((wabi_val) plc)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  old = (wabi_word) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  new = (wabi_word) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }
  res = __sync_bool_compare_and_swap (&plc->val, old, new) ? vm->trh : vm->fls;

  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}


void
wabi_place_builtins(const wabi_vm vm,
                    const wabi_env env)
{
  wabi_defn(vm, env, "plc", &wabi_place_plc);
  if(vm->ert) return;
  wabi_defn(vm, env, "plc?", &wabi_place_plc_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "plc-val", &wabi_place_plc_val);
  if(vm->ert) return;
  wabi_defn(vm, env, "plc-cas", &wabi_place_plc_cas);
}
