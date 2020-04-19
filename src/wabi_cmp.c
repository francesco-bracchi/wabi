#define wabi_cmp_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_map.h"
#include "wabi_number.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_error.h"
#include "wabi_builtin.h"
#include "wabi_cmp.h"


int
wabi_cmp_pair(wabi_pair left, wabi_pair right) {
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}


int
wabi_cmp_fixnum(wabi_fixnum a, wabi_fixnum b) {
  long d = WABI_CAST_INT64(b) - WABI_CAST_INT64(a);
  return d ? (d > 0L ? 1 : -1) : 0;
}


int
wabi_cmp_builtin_combiner(wabi_combiner_builtin a, wabi_combiner_builtin b)
{
  return wabi_binary_cmp((wabi_binary) a->c_name, (wabi_binary) b->c_name);
}


int
wabi_cmp_combiner_derived(wabi_combiner_derived a, wabi_combiner_derived b)
{
    int cmp;

    cmp = wabi_cmp((wabi_val) a->body, (wabi_val) b->body);
    if(cmp) return cmp;

    cmp = wabi_cmp((wabi_val) a->parameters, (wabi_val) b->parameters);
    if(cmp) return cmp;

    cmp = wabi_cmp((wabi_val) a->caller_env_name, (wabi_val) b->caller_env_name);
    if(cmp) return cmp;

    return wabi_cmp((wabi_val) WABI_WORD_VAL(a->static_env),
                    (wabi_val) WABI_WORD_VAL(b->static_env));
}


int
wabi_cmp_cont(wabi_cont a, wabi_cont b)
{
  int cmp;
  // todo short circuit check wabi_cmp_cont over next

  switch(WABI_TAG(a)) {
  case wabi_tag_cont_eval:
    return wabi_cmp((wabi_val) ((wabi_cont_eval) a)->next, (wabi_val) ((wabi_cont_eval) b)->next);

  case wabi_tag_cont_apply:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->args, (wabi_val) ((wabi_cont_apply) b)->args);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->env, (wabi_val) ((wabi_cont_apply) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_apply) a)->next, (wabi_val) ((wabi_cont_apply) b)->next);

  case wabi_tag_cont_call:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->combiner, (wabi_val) ((wabi_cont_call) b)->combiner);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->env, (wabi_val) ((wabi_cont_call) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_call) a)->next, (wabi_val) ((wabi_cont_call) b)->next);

  case wabi_tag_cont_sel:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->left, (wabi_val) ((wabi_cont_sel) b)->left);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->right, (wabi_val) ((wabi_cont_sel) b)->right);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->env, (wabi_val) ((wabi_cont_sel) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_sel) a)->next, (wabi_val) ((wabi_cont_sel) b)->next);

  case wabi_tag_cont_args:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->data, (wabi_val) ((wabi_cont_args) b)->data);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->done, (wabi_val) ((wabi_cont_args) b)->done);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->env, (wabi_val) ((wabi_cont_args) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_args) a)->next, (wabi_val) ((wabi_cont_args) b)->next);

  case wabi_tag_cont_def:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->pattern, (wabi_val) ((wabi_cont_def) b)->pattern);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->env, (wabi_val) ((wabi_cont_def) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_def) a)->next, (wabi_val) ((wabi_cont_def) b)->next);

  case wabi_tag_cont_prog:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->expressions, (wabi_val) ((wabi_cont_prog) b)->expressions);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->env, (wabi_val) ((wabi_cont_prog) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_prog) a)->next, (wabi_val) ((wabi_cont_prog) b)->next);
  default:
    return -1000;
  }
}


int
wabi_cmp(wabi_val a, wabi_val b)
{
  wabi_word tag;
  wabi_word diff;
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;
  // types are different => type order
  tag = WABI_TAG(a);
  diff = tag - WABI_TAG(b);
  if(diff) {
    return (int)(diff >> wabi_word_tag_offset);
  }
  switch(tag) {
  case wabi_tag_constant:
    return (*a - *b);
  case wabi_tag_fixnum:
    return wabi_cmp_fixnum((wabi_fixnum) a, (wabi_fixnum) b);
  case wabi_tag_symbol:
    return wabi_cmp(wabi_symbol_to_binary((wabi_symbol) a), wabi_symbol_to_binary((wabi_symbol) b));
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    return wabi_binary_cmp((wabi_binary) a, (wabi_binary) b);
  case wabi_tag_pair:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    return wabi_map_cmp((wabi_map) a, (wabi_map) b);
  case wabi_tag_env:
    return wabi_env_cmp((wabi_env) a, (wabi_env) b);
  case wabi_tag_app:
  case wabi_tag_oper:
    return wabi_cmp_combiner_derived((wabi_combiner_derived) a, (wabi_combiner_derived) b);
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    return wabi_cmp_builtin_combiner((wabi_combiner_builtin) a, (wabi_combiner_builtin) b);
  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_sel:
  case wabi_tag_cont_args:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    return wabi_cmp_cont((wabi_cont) a, (wabi_cont) b);
  default:
    return *a - *b;
  }
}


int
wabi_eq(wabi_val left, wabi_val right)
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
  case wabi_tag_constant:
  case wabi_tag_fixnum:
    return (*left == *right);
  default:
    return !wabi_cmp(left, right);
  }
}


wabi_error_type
wabi_cmp_eq_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b)) {
      res = wabi_vm_alloc(vm, 1);
      if(!res) return wabi_error_nomem;

      *res = wabi_val_false;
      vm->control = res;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      return wabi_error_none;
    }
  }
  if(*ctrl == wabi_val_nil) {
    res = wabi_vm_alloc(vm, 1);
    if(! res) return wabi_error_nomem;
    *res = wabi_val_true;
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_cmp_gt_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) >= 0) {
      res = wabi_vm_alloc(vm, 1);
      if(!res) return wabi_error_nomem;
      *res = wabi_val_false;
      vm->control = res;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      return wabi_error_none;
    }
    a = b;
  }
  if(*ctrl == wabi_val_nil) {
    res = wabi_vm_alloc(vm, 1);
    if(!res) return wabi_error_nomem;
    *res = wabi_val_true;
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_cmp_lt_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) <= 0) {
      res = wabi_vm_alloc(vm, 1);
      if(!res) return wabi_error_nomem;
      *res = wabi_val_false;
      vm->control = res;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      return wabi_error_none;
    }
    a = b;
  }
  if(*ctrl == wabi_val_nil) {
    res = wabi_vm_alloc(vm, 1);
    if(!res) return wabi_error_nomem;
    *res = wabi_val_true;
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_bindings;
}

// TODO
// rename with eq gt lt since = < > must be for numbers only (maybe not =?)
wabi_error_type
wabi_cmp_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "=", "=", wabi_cmp_eq_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, ">", "gt", wabi_cmp_gt_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, "<", "lt", wabi_cmp_lt_builtin);
  return res;
  /* res = WABI_DEFN(vm, env, ">=", wabi_cmp_gt_builtin); */
  /* res = WABI_DEFN(vm, env, "<=", wabi_cmp_lt_builtin); */
}
