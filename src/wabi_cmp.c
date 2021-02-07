#define wabi_cmp_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_list.h"
#include "wabi_map.h"
#include "wabi_number.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_error.h"
#include "wabi_builtin.h"
#include "wabi_place.h"
#include "wabi_vector.h"
#include "wabi_atom.h"
#include "wabi_cmp.h"


int
wabi_cmp(const wabi_val a, const wabi_val b)
{
  wabi_word tag;
  wabi_word diff;
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;
  // types are different => type order
  tag = WABI_TAG(a);

  diff = tag - WABI_TAG(b);

  switch(tag) {
  case wabi_tag_fixnum:
    if(wabi_is_fixnum(b)) {
      return wabi_cmp_fixnum((wabi_fixnum) a, (wabi_fixnum) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_symbol:
    if (wabi_is_symbol(b)) {
      return wabi_cmp(wabi_symbol_to_binary((wabi_symbol)a),
                      wabi_symbol_to_binary((wabi_symbol)b));
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_atom:
    if (wabi_is_atom(b)) {
      return wabi_cmp(wabi_atom_to_binary((wabi_symbol)a),
                      wabi_atom_to_binary((wabi_symbol)b));
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    if(wabi_is_binary(b)) {
      return wabi_binary_cmp((wabi_binary) a, (wabi_binary) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_pair:
    if(wabi_is_pair(b)) {
      return wabi_pair_cmp((wabi_pair) a, (wabi_pair) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    if (wabi_is_map(b)) {
      return wabi_map_cmp((wabi_map)a, (wabi_map)b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_vector_digit:
  case wabi_tag_vector_deep:
    if(wabi_is_vector(b)) {
      return wabi_vector_cmp((wabi_vector) a, (wabi_vector) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_env:
    if(wabi_is_env(b)) {
      return wabi_env_cmp((wabi_env) a, (wabi_env) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_app:
  case wabi_tag_oper:
    if(WABI_TAG(b) == tag) {
      return wabi_combiner_derived_cmp((wabi_combiner_derived) a, (wabi_combiner_derived) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    if(WABI_TAG(b) == tag) {
      return wabi_combiner_builtin_cmp((wabi_combiner_builtin) a, (wabi_combiner_builtin) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_sel:
  case wabi_tag_cont_args:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    if(WABI_TAG(b) == tag) {
      return wabi_cont_cmp((wabi_cont) a, (wabi_cont) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_place:
    if(wabi_is_place(b)) {
      return wabi_place_cmp((wabi_place) a, (wabi_place) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  default:
    return -10000;
  }
}


int
wabi_eq(const wabi_val left, const wabi_val right)
{
  wabi_word tag;
  wabi_word diff;

  // if the 2 values are the very same, they are equal :|
  if(left == right) return 1;
  // types are different => they are not equal
  tag = WABI_TAG(left);
  diff = tag - WABI_TAG(right);
  if(diff) return 0;

  switch(tag) {
  case wabi_tag_fixnum:
    return (*left == *right);
  default:
    return !wabi_cmp(left, right);
  }
}


static void
wabi_cmp_eq(const wabi_vm vm)
{
  wabi_val a, b, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(wabi_is_pair(ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b)) {
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


/* static void */
/* wabi_cmp_neq(const wabi_vm vm) */
/* { */
/*   wabi_val a, b, ctrl; */

/*   ctrl = vm->ctrl; */
/*   if(!wabi_is_pair(ctrl)) { */
/*     vm->ert = wabi_error_bindings; */
/*     return; */
/*   } */

/*   a = wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   while(wabi_is_pair(ctrl)) { */
/*     b = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(wabi_cmp(a, b)) { */

/*       vm->ctrl = vm->trh; */
/*       vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*       return; */
/*     } */
/*   } */
/*   if(!wabi_atom_is_empty(vm, ctrl)) { */
/*     vm->ert = wabi_error_bindings; */
/*     return; */
/*   } */
/*   vm->ctrl = vm->fls; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/* } */


static void
wabi_cmp_gt(const wabi_vm vm)
{
  wabi_val a, b, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(wabi_is_pair(ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) >= 0) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_cmp_gt_eq(const wabi_vm vm)
{
  wabi_val a, b, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(wabi_is_pair(ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) > 0) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_cmp_lt(const wabi_vm vm)
{
  wabi_val a, b, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(wabi_is_pair(ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) <= 0) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}


static void
wabi_cmp_lt_eq(const wabi_vm vm)
{
  wabi_val a, b, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(wabi_is_pair(ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) < 0) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
}

// TODO
// rename with eq gt lt since = < > must be for numbers only (maybe not =?)
void
wabi_cmp_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "=", &wabi_cmp_eq);
  if(vm->ert) return;
  wabi_defn(vm, env, "/=", &wabi_cmp_eq);
  if(vm->ert) return;
  wabi_defn(vm, env, ">", &wabi_cmp_gt);
  if(vm->ert) return;
  wabi_defn(vm, env, "<", &wabi_cmp_lt);
  if(vm->ert) return;
  wabi_defn(vm, env, ">=", &wabi_cmp_gt_eq);
  if(vm->ert) return;
  wabi_defn(vm, env, "<=", &wabi_cmp_lt_eq);
}
