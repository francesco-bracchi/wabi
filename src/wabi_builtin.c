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
#include "wabi_builtin.h"
#include "wabi_cont.h"
#include "wabi_error.h"


// tmp
#include <stdio.h>
#include "wabi_pr.h"

void
wabi_builtin_def_bt(wabi_vm vm, wabi_env env, wabi_val ps, wabi_val e)
{
  if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE + WABI_CONT_DEF_SIZE)) {
    vm->continuation = (wabi_val) wabi_cont_def_new(vm, env, ps, (wabi_cont) vm->continuation);
    vm->continuation = (wabi_val) wabi_cont_eval_new(vm, env, (wabi_cont) vm->continuation);
    vm->control = e;
    return;
  }
  vm->errno = wabi_error_nomem;
}

WABI_BUILTIN_WRAP2(wabi_builtin_def, wabi_builtin_def_bt)


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


static inline void
wabi_builtin_eval_bt(wabi_vm vm, wabi_env env, wabi_val e, wabi_val x)
{
  if(WABI_IS(wabi_tag_env, e)) {
    if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE)) {
      vm->continuation = (wabi_val) wabi_cont_eval_new(vm, (wabi_env) e, (wabi_cont) vm->continuation);
      vm->control = x;
      return;
    }
    vm->errno = wabi_error_nomem;
  }
  vm->errno = wabi_error_type_mismatch;
}


/* void */
/* wabi_number_builtin_do(wabi_vm vm, wabi_env env) */
/* { */
/*   wabi_val a, ctrl; */

/*   ctrl = vm->control; */

/*   if(WABI_IS(wabi_tag_pair, ctrl)) { */
/*     a = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(WABI_IS(wabi_tag_nil, ctrl)) { */
/*       if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE)) { */
/*         vm->continuation = (wabi_val) wabi_cont_eval_new(vm, (wabi_env) e, (wabi_cont) vm->continuation); */
/*         vm->control = a; */
/*         return; */
/*       } */
/*       vm->errno = wabi_error_nomem; */
/*       return; */
/*     } */
/*     if(WABI_IS(wabi_tag_pair, ctrl)) { */
/*       if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE + WABI_CONT_CALL_SIZE)) { */
/*         vm->continuation = (wabi_val) wabi_cont_eval_new(vm, (wabi_env) e, (wabi_cont) vm->continuation); */
/*         vm->control = a; */
/*         return; */
/*       } */
/*     } */
/* } */



WABI_BUILTIN_WRAP2(wabi_builtin_eval, wabi_builtin_eval_bt);

wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_env env;
  wabi_symbol sym;
  wabi_val val;

  env = wabi_env_new(vm);

  WABI_DEFX(vm, env, "def", "wabi:def", wabi_builtin_def);
  WABI_DEFX(vm, env, "if", "wabi:if", wabi_builtin_if);
  WABI_DEFN(vm, env, "hmap", "wabi:hmap", wabi_builtin_hmap);
  WABI_DEFN(vm, env, "pr", "wabi:pr", wabi_builtin_pr);
  WABI_DEFN(vm, env, "eval", "wabi:eval", wabi_builtin_eval);

  wabi_constant_builtins(vm, env);
  wabi_combiner_builtins(vm, env);
  wabi_pair_builtins(vm, env);
  wabi_number_builtins(vm, env);
  wabi_cmp_builtins(vm, env);

  /* WABI_DEFX(vm, env, "fx", "wabi:fx", wabi_combiner_builtin_fx); */
  /* WABI_DEFN(vm, env, "wrap", "wabi:wrap", wabi_builtin_wrap); */
  /* WABI_DEFN(vm, env, "unwrap", "wabi:unwrap", wabi_builtin_unwrap); */
  /* WABI_DEFN(vm, env, "+", "wabi:+", wabi_number_builtin_sum); */
  /* WABI_DEFN(vm, env, "-", "wabi:-", wabi_number_builtin_diff); */
  /* WABI_DEFN(vm, env, "*", "wabi:*", wabi_number_builtin_mmul); */
  /* WABI_DEFN(vm, env, "/", "wabi:/", wabi_number_builtin_div); */
  /* WABI_DEFN(vm, env, "cons", "wabi:cons", wabi_builtin_cons); */
  /* WABI_DEFN(vm, env, "car", "wabi:car", wabi_builtin_car); */
  /* WABI_DEFN(vm, env, "cdr", "wabi:cdr", wabi_builtin_cdr); */
  /* WABI_DEFN(vm, env, "nil?", "wabi:nil?", wabi_builtin_nil_q); */
  // WABI_DEFN(vm, env, "pair?", "wabi:pair?", wabi_builtin_pair_q);
  // wabi_pr((wabi_val) env);
  return env;
}
