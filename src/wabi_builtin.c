#define wabi_builtin_c

#include <stdlib.h>
#include <time.h>

#include "wabi_value.h"
#include "wabi_pair.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_number.h"
#include "wabi_map.h"
#include "wabi_binary.h"
#include "wabi_reader.h"
#include "wabi_cmp.h"
#include "wabi_vm.h"
#include "wabi_error.h"
#include "wabi_constant.h"


// tmp
#include <stdio.h>
#include "wabi_pr.h"

void
wabi_builtin_fx(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, fs, e, b;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    e = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl) && WABI_IS(wabi_tag_pair, ctrl)) {
      fs = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(WABI_IS(wabi_tag_symbol, e) || *e == wabi_val_ignore) {
        b = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(*ctrl == wabi_val_nil) {
          vm->control = (wabi_val) wabi_combiner_new(vm, env, e, fs, b);
          return;
        }
      }
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "fexpr error");
  }
}

void
wabi_builtin_wrap(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, comb;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    comb = wabi_car((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_oper, comb) || WABI_IS(wabi_tag_bt_oper, comb)) {
      vm->control = (wabi_val) wabi_combiner_wrap(vm, (wabi_combiner) comb);
      return;
    }
    vm->control = (wabi_val) comb;
  }
}


void
wabi_builtin_unwrap(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, comb;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    comb = wabi_car((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_app, comb) || WABI_IS(wabi_tag_bt_app, comb)) {
      vm->control = (wabi_val) wabi_combiner_unwrap(vm, (wabi_combiner) comb);
      return;
    }
    vm->control = (wabi_val) comb;
  }
}


void
wabi_builtin_def(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, fs, e;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    fs = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      e = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(*ctrl == wabi_val_nil) {
        vm->continuation = (wabi_val) wabi_cont_def_new(vm, env, fs, (wabi_cont) vm->continuation);
        vm->continuation = (wabi_val) wabi_cont_eval_new(vm, env, (wabi_cont) vm->continuation);
        vm->control = e;
        return;
      }
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "def error");
}


void
wabi_builtin_if(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, t, l, r;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    t = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      l = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(WABI_IS(wabi_tag_pair, ctrl)) {
        r = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(*ctrl == wabi_val_nil) {
          vm->continuation = (wabi_val) wabi_cont_sel_new(vm, env, l, r, (wabi_cont) vm->continuation);
          vm->continuation = (wabi_val) wabi_cont_eval_new(vm, env, (wabi_cont) vm->continuation);
          vm->control = t;
          return;
        }
      }
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "if error");
}


void
wabi_builtin_hmap(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, k, v;
  wabi_map res;
  ctrl = vm->control;
  res = wabi_map_empty(vm);
  while(*ctrl != wabi_val_nil) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      v = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      res = wabi_map_assoc(vm, res, k, v);
    } else {
      vm->errno = 3;
      vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "map odd number of arguments");
    }
  }
  vm->control = (wabi_val) res;
}


void
wabi_builtin_load(wabi_vm vm, wabi_env env, char* str)
{
  wabi_val exp;

  for(;;) {
    exp = wabi_reader_read_val(vm, &str);
    if(exp == NULL) return;
    vm->control = exp;
    vm->continuation = (wabi_val) wabi_cont_eval_new(vm, env, (wabi_cont) vm->continuation);
    wabi_vm_run(vm);
    if(vm->errno) return;
  }
}




void
wabi_builtin_nil_q(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, val, res;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      res = (wabi_val) wabi_vm_alloc(vm, 1);
      *res = *val == wabi_val_nil ? wabi_val_true : wabi_val_false;
      vm->control = res;
      return;
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "nil?: wrong number of arguments");
}

void
wabi_builtin_eq(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, res, l, r;

  ctrl = vm->control;
  res = (wabi_val) wabi_vm_alloc(vm, 1);

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    l = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    while(WABI_IS(wabi_tag_pair, ctrl)) {
      r = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(wabi_cmp(l, r)) {
        *res = wabi_val_false;
        vm->control = res;
        return;
      }
    }
    if(*ctrl == wabi_val_nil) {
      *res = wabi_val_true;
      vm->control = res;
      return;
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "=: wrong number of arguments");
}

void
wabi_builtin_gt(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, res, l, r;

  ctrl = vm->control;
  res = (wabi_val) wabi_vm_alloc(vm, 1);

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    l = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    while(WABI_IS(wabi_tag_pair, ctrl)) {
      r = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      printf("cmp: %i\n", wabi_cmp(l, r));
      if(wabi_cmp(l, r) < 0) {
        *res = wabi_val_false;
        vm->control = res;
        return;
      }
    }
    if(*ctrl == wabi_val_nil) {
      *res = wabi_val_true;
      vm->control = res;
      return;
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, ">: wrong number of arguments");
}


void
wabi_builtin_pr(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, v;

  ctrl = vm->control;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    vm->control = wabi_car((wabi_pair) ctrl);
    wabi_pr(vm->control);
    printf("\n");
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
}


#define SYM(vm, str)                                                    \
  wabi_symbol_new(vm,                                                   \
                  (wabi_val) wabi_binary_leaf_new_from_cstring(vm, str))

#define BTOPER(vm, str, fun)                                            \
  wabi_operator_builtin_new(vm,                                         \
                            (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, str), \
                            fun)                                        \

#define BTAPP(vm, str, fun)                                             \
  wabi_application_builtin_new(vm,                                      \
                               (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, str), \
                               fun)                                     \


#define WABI_DEFX(vm, env, name, btname, fun)                           \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) BTOPER(vm, btname, fun)                       \
               );

#define WABI_DEFN(vm, env, name, btname, fun)                           \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) BTAPP(vm, btname, fun)                        \
               );

#define WABI_DEF(vm, env, name, val)                                    \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) val                                           \
               );

wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_env env;
  wabi_symbol sym;
  wabi_val val;

  env = wabi_env_new(vm);

  val = (wabi_val) wabi_vm_alloc(vm, 5);

  *val = wabi_val_nil;
  WABI_DEF(vm, env, "nil", val);
  val++;

  *val = wabi_val_true;
  WABI_DEF(vm, env, "true", val);
  val++;

  *val = wabi_val_false;
  WABI_DEF(vm, env, "false", val);
  val++;

  *val = wabi_val_ignore;
  WABI_DEF(vm, env, "ignore", val);
  val++;

  *val = wabi_val_zero;
  WABI_DEF(vm, env, "zero", val);

  WABI_DEFX(vm, env, "def", "wabi:def", wabi_builtin_def);
  WABI_DEFX(vm, env, "fx", "wabi:fx", wabi_builtin_fx);
  WABI_DEFX(vm, env, "if", "wabi:if", wabi_builtin_if);
  WABI_DEFN(vm, env, "wrap", "wabi:wrap", wabi_builtin_wrap);
  WABI_DEFN(vm, env, "unwrap", "wabi:unwrap", wabi_builtin_unwrap);
  WABI_DEFN(vm, env, "+", "wabi:+", wabi_number_sum_builtin);
  WABI_DEFN(vm, env, "-", "wabi:-", wabi_number_diff_builtin);
  WABI_DEFN(vm, env, "*", "wabi:*", wabi_number_mul_builtin);
  WABI_DEFN(vm, env, "/", "wabi:/", wabi_number_div_builtin);
  WABI_DEFN(vm, env, "hmap", "wabi:hmap", wabi_builtin_hmap);
  /* WABI_DEFN(vm, env, "cons", "wabi:cons", wabi_builtin_cons); */
  /* WABI_DEFN(vm, env, "car", "wabi:car", wabi_builtin_car); */
  /* WABI_DEFN(vm, env, "cdr", "wabi:cdr", wabi_builtin_cdr); */
  WABI_DEFN(vm, env, "nil?", "wabi:nil?", wabi_builtin_nil_q);
  // WABI_DEFN(vm, env, "pair?", "wabi:pair?", wabi_builtin_pair_q);
  WABI_DEFN(vm, env, "=", "wabi:=", wabi_builtin_eq);
  WABI_DEFN(vm, env, ">", "wabi:>", wabi_builtin_gt);
  // not primitive
  WABI_DEFN(vm, env, "pr", "wabi:pr", wabi_builtin_pr);

  return env;
}
