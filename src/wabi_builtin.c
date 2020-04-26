#define wabi_builtin_c

#include <stdlib.h>
#include <stdio.h>
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
#include "wabi_delim.h"
#include "wabi_error.h"
#include "wabi_pr.h"
#include "wabi_map.h"
#include "wabi_hash.h"


wabi_error_type
wabi_builtin_def_bt(wabi_vm vm, wabi_val ps, wabi_val e)
{
  wabi_env env;
  wabi_cont cont;

  env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
  cont = wabi_cont_next((wabi_cont) vm->continuation);
  cont = wabi_cont_push_def(vm, env, ps, cont);
  if(cont) {
    cont = wabi_cont_push_eval(vm, cont);
    if(cont) {
      vm->control = e;
      vm->env = (wabi_val) env;
      vm->continuation = (wabi_val) cont;
      return wabi_error_none;
    }
  }
  return wabi_error_nomem;
}


WABI_BUILTIN_WRAP2(wabi_builtin_def, wabi_builtin_def_bt)


wabi_error_type
wabi_builtin_if(wabi_vm vm)
{
  wabi_env env;
  wabi_val ctrl, h, i, l, self;
  wabi_cont cont;

  ctrl = vm->control;
  self = (wabi_val) ((wabi_cont_call) vm->continuation)->combiner;
  env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;

  if(*ctrl == wabi_val_nil) {
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    h = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(*ctrl == wabi_val_nil) {
      cont = wabi_cont_next((wabi_cont) vm->continuation);
      cont = wabi_cont_push_eval(vm, cont);
      if(cont) {
        vm->control = h;
        vm->continuation = (wabi_val) cont;
        vm->env = (wabi_val) env;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      i = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      cont = wabi_cont_next((wabi_cont) vm->continuation);
      l = (wabi_val) wabi_cons(vm, self, ctrl);
      cont = wabi_cont_push_sel(vm, env, i, l, cont);
      if(cont) {
        cont = wabi_cont_push_eval(vm, cont);
        if(cont) {
          vm->control = h;
          vm->env = (wabi_val) env;
          vm->continuation = (wabi_val) cont;
          return wabi_error_none;
        }
      }
      return wabi_error_nomem;
    }
  }
  return wabi_error_type_mismatch;
}

/* wabi_error_type */
/* wabi_builtin_if(wabi_vm vm) */
/* { */
/*   wabi_env env; */
/*   wabi_val ctrl, t, l, r; */
/*   wabi_cont cont; */

/*   ctrl = vm->control; */
/*   if(WABI_IS(wabi_tag_pair, ctrl)) { */
/*     t = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(WABI_IS(wabi_tag_pair, ctrl)) { */
/*       l = wabi_car((wabi_pair) ctrl); */
/*       ctrl = wabi_cdr((wabi_pair) ctrl); */
/*       if(WABI_IS(wabi_tag_pair, ctrl)) { */
/*         r = wabi_car((wabi_pair) ctrl); */
/*         ctrl = wabi_cdr((wabi_pair) ctrl); */
/*         if(*ctrl == wabi_val_nil) { */
/*           env = (wabi_env) ((wabi_cont_call) vm->continuation)->env; */
/*           cont = wabi_cont_next((wabi_cont) vm->continuation); */
/*           cont = wabi_cont_push_sel(vm, env, l, r, cont); */
/*           if(cont) { */
/*             cont = wabi_cont_push_eval(vm, cont); */
/*             if(cont) { */
/*               vm->control = t; */
/*               vm->env = (wabi_val) env; */
/*               vm->continuation = (wabi_val) cont; */
/*               return wabi_error_none; */
/*             } */
/*           } */
/*           return wabi_error_nomem; */
/*         } */
/*       } */
/*     } */
/*   } */
/*   return wabi_error_type_mismatch; */
/* } */


inline static wabi_val
wabi_builtin_reverse(wabi_vm vm, wabi_val es) {
  wabi_val xs;
  xs = wabi_vm_alloc(vm, 1);
  *xs = wabi_val_nil;

  while(*es != wabi_val_nil) {
    xs = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) es), xs);
    es = wabi_cdr((wabi_pair) es);
    if(!xs) return NULL;
  }
  return xs;
}


wabi_error_type
wabi_builtin_load(wabi_vm vm, wabi_env env, char* str)
{
  wabi_val exp, es, xs;
  wabi_cont cont;
  /// TODO invert here
  es = wabi_vm_alloc(vm, 1);
  *es = wabi_val_nil;
  do {
    exp = wabi_reader_read_val(vm, &str);
    if(exp) {
      xs = (wabi_val) wabi_cons(vm, exp, es);
      if(xs) {
        es = (wabi_val) xs;
        continue;
      }
      return wabi_error_nomem;
    }
    if(*str) {
      printf("THREE\n");
      return wabi_error_other;
    }
  } while(*str);
  es = wabi_builtin_reverse(vm, es);
  cont = NULL;
  if(es) {
    if(vm->continuation) {
      cont = wabi_cont_next((wabi_cont) vm->continuation);
    }
    cont = wabi_cont_push_prog(vm, env, wabi_cdr((wabi_pair) es), cont);
    if(cont) {
      cont = wabi_cont_push_eval(vm, cont);
      if(cont) {
        vm->control = wabi_car((wabi_pair) es);
        vm->env = (wabi_val) env;
        vm->continuation = (wabi_val) cont;
        return wabi_error_none;
      }
    }
  }
  return wabi_error_nomem;
}


wabi_error_type
wabi_builtin_eq(wabi_vm vm)
{
  wabi_val ctrl, res, l, r;

  ctrl = vm->control;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      l = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        r = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(wabi_cmp(l, r)) {
          *res = wabi_val_false;
          vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
          vm->control = res;
          return wabi_error_none;
        }
      }
      if(*ctrl == wabi_val_nil) {
        *res = wabi_val_true;
        vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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

  ctrl = vm->control;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      l = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        r = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(wabi_cmp(l, r) < 0) {
          *res = wabi_val_false;
          vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
          vm->control = res;
          return wabi_error_none;
        }
      }
      if(*ctrl == wabi_val_nil) {
        *res = wabi_val_true;
        vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
  wabi_val ctrl;

  ctrl = vm->control;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    vm->control = wabi_car((wabi_pair) ctrl);
    wabi_pr(vm->control);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) printf(" ");
  }
  vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
  printf("\n");
  return wabi_error_none;
}


// TODO: support variadic calls
static inline wabi_error_type
wabi_builtin_eval_bt(wabi_vm vm, wabi_val e, wabi_val x)
{
  wabi_cont cont;
  if(WABI_IS(wabi_tag_env, e)) {
    cont = wabi_cont_next((wabi_cont) vm->continuation);
    cont = wabi_cont_push_eval(vm, cont);
    if(cont) {
      vm->control = x;
      vm->env = (wabi_val) e;
      vm->continuation = (wabi_val) cont;
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
  wabi_cont cont;

  ctrl = vm->control;

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(*ctrl == wabi_val_nil) {
      env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
      cont = wabi_cont_next((wabi_cont) vm->continuation);
      cont = wabi_cont_push_eval(vm, cont);
      if(! cont) return wabi_error_nomem;
      vm->control = a;
      vm->env = (wabi_val) env;
      vm->continuation = (wabi_val) cont;
      return wabi_error_none;
    }
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
      cont = wabi_cont_next((wabi_cont) vm->continuation);
      cont = wabi_cont_push_prog(vm, env, ctrl, cont);
      if(cont) {
        cont = wabi_cont_push_eval(vm, cont);
        if(cont) {
          vm->control = a;
          vm->env = (wabi_val) env;
          vm->continuation = (wabi_val) cont;
          return wabi_error_none;
        }
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
  clock_t t;

  if(*(vm->control) == wabi_val_nil) {
    t = clock();
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = (t * 1000000 / CLOCKS_PER_SEC) & wabi_word_value_mask;
      WABI_SET_TAG(res, wabi_tag_fixnum);
      vm->control = res;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_type_mismatch;
}


static inline wabi_error_type
wabi_builtin_not_bt(wabi_vm vm, wabi_val e) {
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = (*e == wabi_val_false || *e == wabi_val_nil) ? wabi_val_true : wabi_val_false;
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_builtin_not, wabi_builtin_not_bt);


static inline wabi_error_type
wabi_builtin_sym_qmark_bt(wabi_vm vm, wabi_val e) {
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = WABI_IS(wabi_tag_symbol, e) ? wabi_val_true : wabi_val_false;
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_builtin_sym_qmark, wabi_builtin_sym_qmark_bt);

static inline wabi_error_type
wabi_builtin_hash_bt(wabi_vm vm, wabi_val v) {
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = wabi_hash(v);
    WABI_SET_TAG(res, wabi_tag_fixnum);
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_builtin_hash, wabi_builtin_hash_bt);

wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_error_type res;
  wabi_env env;

  env = wabi_env_new(vm);

  res = WABI_DEFX(vm, env, "def", "def", wabi_builtin_def);
  if(res) return NULL;
  res = WABI_DEFX(vm, env, "if", "if", wabi_builtin_if);
  if(res) return NULL;
  res = WABI_DEFX(vm, env, "do", "do", wabi_builtin_do);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "pr", "pr", wabi_builtin_pr);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "eval", "eval", wabi_builtin_eval);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "clock", "clock", wabi_builtin_clock);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "not", "not", wabi_builtin_not);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "sym?", "sym?", wabi_builtin_sym_qmark);
  if(res) return NULL;
  res = WABI_DEFN(vm, env, "hash", "hash", wabi_builtin_hash);
  if(res) return NULL;

  res = wabi_constant_builtins(vm, env);
  if(res) return NULL;
  res = wabi_combiner_builtins(vm, env);
  if(res) return NULL;
  res = wabi_pair_builtins(vm, env);
  if(res) return NULL;
  res = wabi_number_builtins(vm, env);
  if(res) return NULL;
  res = wabi_binary_builtins(vm, env);
  if(res) return NULL;
  res = wabi_cmp_builtins(vm, env);
  if(res) return NULL;
  res = wabi_env_builtins(vm, env);
  if(res) return NULL;
  res = wabi_delim_builtins(vm, env);
  if(res) return NULL;
  res = wabi_map_builtins(vm, env);
  if(res) return NULL;

  return env;
}
