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

wabi_error_type
wabi_builtin_def_bt(wabi_vm vm, wabi_val ps, wabi_val e)
{
  wabi_env env;
  if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE + WABI_CONT_DEF_SIZE)) {
    env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
    wabi_cont_pop(vm);
    wabi_cont_push_def(vm, env, ps);
    wabi_cont_push_eval(vm, env);
    vm->control = e;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP2(wabi_builtin_def, wabi_builtin_def_bt)

wabi_error_type
wabi_builtin_if(wabi_vm vm)
{
  wabi_env env;
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
          if(wabi_vm_has_rooms(vm, WABI_CONT_SEL_SIZE + WABI_CONT_EVAL_SIZE)) {
            env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
            wabi_cont_pop(vm);
            wabi_cont_push_sel(vm, env, l, r);
            wabi_cont_push_eval(vm, env);
            vm->control = t;
            return wabi_error_none;
          }
          return wabi_error_nomem;
        }
      }
    }
  }
  return wabi_error_type_mismatch;
}


wabi_error_type
wabi_builtin_hmap(wabi_vm vm)
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
      if(!res) return wabi_error_nomem;
    } else {
      return wabi_error_type_mismatch;
    }
  }
  vm->control = (wabi_val) res;
  return wabi_error_nomem;
}


wabi_error_type
wabi_builtin_load(wabi_vm vm, wabi_env env, char* str)
{
  wabi_val exp;
  wabi_error_type e;

  for(;;) {
    exp = wabi_reader_read_val(vm, &str);
    /* wabi_pr(exp); */
    /* printf("\n"); */
    // gc before...
    if(exp == NULL) {
      exp = wabi_reader_read_val(vm, &str);
      if(exp == NULL) return wabi_error_nomem;
    }
    vm->control = exp;
    if(vm->continuation) wabi_cont_pop(vm);
    wabi_cont_push_eval(vm, env);
    e = wabi_vm_run(vm);
    if(e == wabi_error_done)
      continue;
        if(e) {
        printf("foo %i\n", e);
        return e;
        }
  }
  return wabi_error_none;
}


wabi_error_type
wabi_builtin_eq(wabi_vm vm)
{
  wabi_val ctrl, res, l, r;

  ctrl = vm->control;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = (wabi_val) wabi_vm_alloc(vm, 1);

    if(WABI_IS(wabi_tag_pair, ctrl)) {
      l = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        r = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(wabi_cmp(l, r)) {
          *res = wabi_val_false;
          wabi_cont_pop(vm);
          vm->control = res;
          return wabi_error_none;
        }
      }
      if(*ctrl == wabi_val_nil) {
        *res = wabi_val_true;
        wabi_cont_pop(vm);
        vm->control = res;
        return wabi_error_none;
      }
    }
    return wabi_error_type_mismatch;
  }
  return wabi_error_nomem;
}


wabi_error_type
wabi_builtin_gt(wabi_vm vm)
{
  wabi_val ctrl, res, l, r;

  if(wabi_vm_has_rooms(vm, 1)) {
    ctrl = vm->control;
    res = (wabi_val) wabi_vm_alloc(vm, 1);

    if(WABI_IS(wabi_tag_pair, ctrl)) {
      l = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        r = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(wabi_cmp(l, r) < 0) {
          *res = wabi_val_false;
          wabi_cont_pop(vm);
          vm->control = res;
          return wabi_error_none;
        }
      }
      if(*ctrl == wabi_val_nil) {
        *res = wabi_val_true;
        wabi_cont_pop(vm);
        vm->control = res;
        return wabi_error_none;
      }
    }
    return wabi_error_type_mismatch;
  }
  return wabi_error_nomem;
}


wabi_error_type
wabi_builtin_pr(wabi_vm vm)
{
  wabi_val ctrl, v;

  ctrl = vm->control;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    vm->control = wabi_car((wabi_pair) ctrl);
    wabi_pr(vm->control);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) printf(" ");
  }
  wabi_cont_pop(vm);
  printf("\n");
  return wabi_error_none;
}


// TODO: support variadic calls
static inline wabi_error_type
wabi_builtin_eval_bt(wabi_vm vm, wabi_val e, wabi_val x)
{
  if(WABI_IS(wabi_tag_env, e)) {
    if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE)) {
      wabi_cont_pop(vm);
      wabi_cont_push_eval(vm, (wabi_env) e);
      vm->control = x;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_type_mismatch;
}


wabi_error_type
wabi_builtin_do(wabi_vm vm)
{
  wabi_val a, ctrl;
  wabi_env env;

  ctrl = vm->control;

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      if(wabi_vm_has_rooms(vm,WABI_CONT_EVAL_SIZE)) {
        env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
        wabi_cont_pop(vm);
        wabi_cont_push_eval(vm, env);
        vm->control = a;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      if(wabi_vm_has_rooms(vm, WABI_CONT_EVAL_SIZE + WABI_CONT_PROG_SIZE)) {
        env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
        wabi_cont_pop(vm);
        wabi_cont_push_prog(vm, (wabi_env) env, wabi_cdr((wabi_pair) ctrl));
        wabi_cont_push_eval(vm, (wabi_env) env);
        vm->control = a;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
  }
  return wabi_error_type_mismatch;
}



WABI_BUILTIN_WRAP2(wabi_builtin_eval, wabi_builtin_eval_bt);


wabi_error_type
wabi_builtin_clock(wabi_vm vm)
{
  wabi_val res;
  if(*(vm->control) == wabi_val_nil) {
    if(wabi_vm_has_rooms(vm, 1)) {
      clock_t t = clock();
      res = wabi_vm_alloc(vm, 1);
      *res = (t * 1000000 / CLOCKS_PER_SEC) & wabi_word_value_mask;
      WABI_SET_TAG(res, wabi_tag_fixnum);
      vm->control = res;
      wabi_cont_pop(vm);
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_type_mismatch;
}


wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_error_type res;
  wabi_env env;

  env = wabi_env_new(vm);

  res = WABI_DEFX(vm, env, "def", "wabi:def", wabi_builtin_def);
  if(res) return NULL;
  res = WABI_DEFX(vm, env, "if", "wabi:if", wabi_builtin_if);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "hmap", "wabi:hmap", wabi_builtin_hmap);
  if(res) return NULL;
  res = WABI_DEFX(vm, env, "do", "wabi:do", wabi_builtin_do);
  if(res) return NULL;
  res = WABI_DEFX(vm, env, "if", "wabi:if", wabi_builtin_if);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "pr", "wabi:pr", wabi_builtin_pr);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "eval", "wabi:eval", wabi_builtin_eval);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "clock", "wabi:clock", wabi_builtin_clock);
  if(res) return NULL;

  res = wabi_constant_builtins(vm, env);
  if(res) return NULL;
  res = wabi_combiner_builtins(vm, env);
  if(res) return NULL;
  res = wabi_pair_builtins(vm, env);
  if(res) return NULL;
  res = wabi_number_builtins(vm, env);
  if(res) return NULL;
  res = wabi_cmp_builtins(vm, env);
  if(res) return NULL;
  res = wabi_env_builtins(vm, env);
  if(res) return NULL;

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
