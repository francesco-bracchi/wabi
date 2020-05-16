#define wabi_place_c

#include <stdio.h>

#include "wabi_place.h"
#include "wabi_error.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

static inline wabi_word
wabi_place_uid(wabi_place place)
{
  static wabi_word wabi_place_cnt = 0;
  return ((wabi_word) place) ^ (++wabi_place_cnt);
}


wabi_place
wabi_place_new(wabi_vm vm, wabi_val init)
{
  wabi_place place = (wabi_place) wabi_vm_alloc(vm, WABI_PLACE_SIZE);
  if(place) {
    place->uid = wabi_place_uid(place);
    place->val = (wabi_word) init;
    WABI_SET_TAG(place, wabi_tag_place);
  }
  return place;
}


static wabi_error_type
wabi_place_plc(wabi_vm vm)
{
  wabi_val ctrl, init;
  wabi_place res;
  ctrl = vm->ctrl;
  if(wabi_is_nil(ctrl)) {
    init = vm->nil;
  }
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    init = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  if(!wabi_is_nil(ctrl)) {
    return wabi_error_bindings;
  }
  res = wabi_place_new(vm, init);
  if(! res)
    return wabi_error_nomem;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_place_plc_p(wabi_vm vm)
{
  wabi_val res, ctrl, plc;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    plc = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! WABI_IS(wabi_tag_place, plc)) {
      *res = wabi_val_false;
      vm->ctrl = res;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return wabi_error_none;
    }
  }
  if(!wabi_is_nil(ctrl)) return wabi_error_bindings;
  *res = wabi_val_true;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}

static wabi_error_type
wabi_place_plc_val(wabi_vm vm)
{
  wabi_val ctrl;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! WABI_IS(wabi_tag_place, plc))
    return wabi_error_type_mismatch;

  vm->ctrl = wabi_place_val(plc);
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_place_plc_cas(wabi_vm vm)
{
  wabi_val ctrl, val, old, new, res;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! WABI_IS(wabi_tag_place, plc))
    return wabi_error_type_mismatch;

  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  old = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  new = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_is_nil(ctrl))
    return wabi_error_bindings;

  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  *res = __sync_bool_compare_and_swap (&plc->val, old, new) ? wabi_val_true : wabi_val_false;

  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
  return wabi_error_none;
}


wabi_error_type
wabi_place_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "plc", "plc", wabi_place_plc);
  if(res) return res;
  res = WABI_DEFN(vm, env, "plc?", "plc", wabi_place_plc_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "plc-val", "plc-val", wabi_place_plc_val);
  if(res) return res;
  res = WABI_DEFN(vm, env, "plc-cas", "plc-cas", wabi_place_plc_cas);
  return res;
}
