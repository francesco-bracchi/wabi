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
wabi_cmp_leaves(wabi_binary_leaf left,
                wabi_word from_left,
                wabi_word len_left,
                wabi_binary_leaf right,
                wabi_word from_right,
                wabi_word len_right)
{
  wabi_word count = (len_left < len_right ? len_left : len_right) - 1;
  char* left_char = ((char *) left->data_ptr) + from_left;
  char* right_char = ((char *) right->data_ptr) + from_right;
  while(*left_char == *right_char && count) {
    count--;
    left_char++;
    right_char++;
  }
  return *left_char == *right_char ? len_left - len_right : *left_char - *right_char;
}


int
wabi_cmp_bin(wabi_binary left,
             wabi_word from_left,
             wabi_word len_left,
             wabi_binary right,
             wabi_word from_right,
             wabi_word len_right)
{
  if(WABI_TAG((wabi_val) left) == wabi_tag_bin_node) {
    wabi_binary left_left = (wabi_binary) ((wabi_binary_node) left)->left;
    wabi_binary left_right = (wabi_binary) ((wabi_binary_node) left)->right;
    wabi_word pivot = wabi_binary_length(left_left);
    if(from_left >= pivot) {
      return wabi_cmp_bin(left_right, from_left - pivot, len_left, right, from_right, len_right);
    }
    wabi_word left_len0 = pivot - from_left;
    if(len_left <= left_len0) {
      // is this ever visited?
      return wabi_cmp_bin(left_left, from_left, len_left, right, from_right, len_right);
    }
    if(len_right <= left_len0) {
      return wabi_cmp_bin(left_left, from_left, left_len0, right, from_right, len_right);
    }
    int cmp0 = wabi_cmp_bin(left_left, from_left, left_len0, right, from_right, left_len0);
    if(cmp0) return cmp0;
    return wabi_cmp_bin(left_right, 0, len_left - left_len0, right, from_right + left_len0, len_right - left_len0);
  }

  if(WABI_TAG((wabi_val) right) == wabi_tag_bin_node) {
    return - wabi_cmp_bin(right, from_right, len_right, left, from_left, len_left);
  }
  return wabi_cmp_leaves((wabi_binary_leaf) left, from_left, len_left,
                         (wabi_binary_leaf) right, from_right, len_right);
}


int
wabi_cmp_binary(wabi_binary left, wabi_binary right)
{
  return wabi_cmp_bin(left, 0, wabi_binary_length(left), right, 0, wabi_binary_length(right));
}

int
wabi_cmp_pair(wabi_pair left, wabi_pair right) {
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}


int
wabi_cmp_map(wabi_map left, wabi_map right)
{
  wabi_map_iter_t left_iter, right_iter;
  wabi_map_entry left_entry, right_entry;
  int cmp;
  wabi_map_iterator_init(&left_iter, left);
  wabi_map_iterator_init(&right_iter, right);

  do {
    left_entry = wabi_map_iterator_current(&left_iter);
    right_entry = wabi_map_iterator_current(&right_iter);

    if(!left_entry && !right_entry) {
      return 0;
    }
    else if(!right_entry) {
      return 1;
    }
    else if(!left_entry) {
      return -1;
    }
    else {
      cmp = wabi_cmp(WABI_MAP_ENTRY_KEY(left_entry),
                     WABI_MAP_ENTRY_KEY(right_entry));
      if(cmp) return cmp;
      cmp =  wabi_cmp(WABI_MAP_ENTRY_VALUE(left_entry),
                      WABI_MAP_ENTRY_VALUE(right_entry));
      if(cmp) return cmp;
    }
    wabi_map_iterator_next(&left_iter);
    wabi_map_iterator_next(&right_iter);
  } while(1);
}


int
wabi_cmp_env(wabi_env left, wabi_env right) {
  int cmp0;
  do {
    cmp0 = wabi_cmp_map((wabi_map) left->data, (wabi_map) right->data);
    if(cmp0) return cmp0;

    left = (wabi_env) WABI_WORD_VAL(left->prev);
    right = (wabi_env) WABI_WORD_VAL(right->prev);
    if(left == right) return 0;
    if(left == NULL) return 1;
    if(right == NULL) return -1;
  } while(1);
}


int
wabi_cmp_fixnum(wabi_fixnum a, wabi_fixnum b) {
  long d = WABI_CAST_INT64(b) - WABI_CAST_INT64(a);
  return d ? (d > 0L ? 1 : -1) : 0;
}


int
wabi_cmp_builtin_combiner(wabi_combiner_builtin a, wabi_combiner_builtin b)
{
  return wabi_cmp_binary((wabi_binary) a->c_name, (wabi_binary) b->c_name);
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
  // todo short circuit check wabi_cmp_cont over prev

  switch(WABI_TAG(a)) {
  case wabi_tag_cont_eval:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_eval) a)->env, (wabi_val) ((wabi_cont_eval) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_eval) a)->prev, (wabi_val) ((wabi_cont_eval) b)->prev);

  case wabi_tag_cont_apply:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->args, (wabi_val) ((wabi_cont_apply) b)->args);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->env, (wabi_val) ((wabi_cont_apply) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_apply) a)->prev, (wabi_val) ((wabi_cont_apply) b)->prev);

  case wabi_tag_cont_call:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->combiner, (wabi_val) ((wabi_cont_call) b)->combiner);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->env, (wabi_val) ((wabi_cont_call) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_call) a)->prev, (wabi_val) ((wabi_cont_call) b)->prev);

  case wabi_tag_cont_sel:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->left, (wabi_val) ((wabi_cont_sel) b)->left);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->right, (wabi_val) ((wabi_cont_sel) b)->right);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->env, (wabi_val) ((wabi_cont_sel) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_sel) a)->prev, (wabi_val) ((wabi_cont_sel) b)->prev);

  case wabi_tag_cont_eval_more:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_eval_more) a)->data, (wabi_val) ((wabi_cont_eval_more) b)->data);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_eval_more) a)->done, (wabi_val) ((wabi_cont_eval_more) b)->done);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_eval_more) a)->env, (wabi_val) ((wabi_cont_eval_more) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_eval_more) a)->prev, (wabi_val) ((wabi_cont_eval_more) b)->prev);

  case wabi_tag_cont_def:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->pattern, (wabi_val) ((wabi_cont_def) b)->pattern);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->env, (wabi_val) ((wabi_cont_def) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_def) a)->prev, (wabi_val) ((wabi_cont_def) b)->prev);

  case wabi_tag_cont_prog:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->expressions, (wabi_val) ((wabi_cont_prog) b)->expressions);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->env, (wabi_val) ((wabi_cont_prog) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_prog) a)->prev, (wabi_val) ((wabi_cont_prog) b)->prev);
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
    return wabi_cmp(WABI_DEREF(a), WABI_DEREF(b));
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    return wabi_cmp_binary((wabi_binary) a, (wabi_binary) b);
  case wabi_tag_pair:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    return wabi_cmp_map((wabi_map) a, (wabi_map) b);
  case wabi_tag_env:
    return wabi_cmp_env((wabi_env) a, (wabi_env) b);
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
  case wabi_tag_cont_eval_more:
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
  return !wabi_cmp(left, right);
}


wabi_error_type
wabi_cmp_eq_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b)) {
      if(wabi_vm_has_rooms(vm, 1)) {
        res = wabi_vm_alloc(vm, 1);
        *res = wabi_val_false;
        vm->control = res;
        wabi_cont_pop(vm);
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
  }
  if(*ctrl == wabi_val_nil) {
    if(wabi_vm_has_rooms(vm, 1)) {
      res = wabi_vm_alloc(vm, 1);
      *res = wabi_val_true;
      vm->control = res;
      wabi_cont_pop(vm);
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_cmp_gt_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) >= 0) {
      if(wabi_vm_has_rooms(vm, 1)) {
        res = wabi_vm_alloc(vm, 1);
        *res = wabi_val_false;
        vm->control = res;
        wabi_cont_pop(vm);
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
  }
  if(*ctrl == wabi_val_nil) {
    if(wabi_vm_has_rooms(vm, 1)) {
      res = wabi_vm_alloc(vm, 1);
      *res = wabi_val_true;
      vm->control = res;
      wabi_cont_pop(vm);
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_cmp_lt_builtin(wabi_vm vm)
{
  wabi_val a, b, ctrl, res;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    b = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(wabi_cmp(a, b) <= 0) {
      if(wabi_vm_has_rooms(vm, 1)) {
        res = wabi_vm_alloc(vm, 1);
        *res = wabi_val_false;
        vm->control = res;
        wabi_cont_pop(vm);
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
  }
  if(*ctrl == wabi_val_nil) {
    if(wabi_vm_has_rooms(vm, 1)) {
      res = wabi_vm_alloc(vm, 1);
      *res = wabi_val_true;
      vm->control = res;
      wabi_cont_pop(vm);
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_bindings;
}


wabi_error_type
wabi_cmp_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "=", "wabi:=", wabi_cmp_eq_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, ">", "wabi:gt", wabi_cmp_gt_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, "<", "wabi:lt", wabi_cmp_lt_builtin);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, ">=", wabi_cmp_gt_builtin); */
  /* res = WABI_DEFN(vm, env, "<=", wabi_cmp_lt_builtin); */
}
