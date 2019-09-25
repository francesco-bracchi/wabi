#define wabi_builtin_c

#include "wabi_value.h"
#include "wabi_pair.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_number.h"
#include "wabi_vm.h"

void
wabi_builtin_sum(wabi_vm vm, wabi_env env)
{
  // todo: check types (arguments and intes)
  // todo: handle overlflows
  long res;
  wabi_val ctrl, c;
  ctrl = vm->control;
  res = 0;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    c = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_fixnum, c)) {
      res += WABI_CAST_INT64((wabi_fixnum) c);
      continue;
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
    return;
  }
  vm->control = wabi_fixnum_new(vm, res);
}

void
wabi_builtin_diff(wabi_vm vm, wabi_env env)
{
  // todo: check types (arguments and intes)
  // todo: handle overlflows
  long res;
  wabi_val ctrl, c;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    c = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_fixnum, c)) {
      res = WABI_CAST_INT64((wabi_fixnum) c);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        c = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(WABI_IS(wabi_tag_fixnum, c)) {
          res -= WABI_CAST_INT64((wabi_fixnum) c);
          continue;
        }
        vm->errno = 1;
        vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
        return;
      }
      vm->control = wabi_fixnum_new(vm, res);
      return;
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
    return;
  }
}


void
wabi_builtin_mul(wabi_vm vm, wabi_env env)
{
  // todo: check types (arguments and intes)
  // todo: handle overlflows
  long res;
  wabi_val ctrl, c;
  ctrl = vm->control;
  res = 0;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    c = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_fixnum, c)) {
      res *= WABI_CAST_INT64((wabi_fixnum) c);
      continue;
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
    return;
  }
  vm->control = wabi_fixnum_new(vm, res);
}


void
wabi_builtin_div(wabi_vm vm, wabi_env env)
{
  // todo: check types (arguments and intes)
  // todo: handle overlflows
  long res, x;
  wabi_val ctrl, c;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    c = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_fixnum, c)) {
      res = WABI_CAST_INT64((wabi_fixnum) c);
      while(WABI_IS(wabi_tag_pair, ctrl)) {
        c = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(WABI_IS(wabi_tag_fixnum, c)) {
          x = WABI_CAST_INT64((wabi_fixnum) c);
          if(x != 0) {
            res /= WABI_CAST_INT64((wabi_fixnum) c);
            continue;
          }
          vm->errno = 1;
          vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "division by zero");
        }
        vm->errno = 1;
        vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
        return;
      }
      vm->control = wabi_fixnum_new(vm, res);
      return;
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "sum not a number");
    return;
  }
}


void
wabi_builtin_fx(wabi_vm vm, wabi_env env)
{
  wabi_val ctrl, fs, e, b;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    fs = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      e = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(WABI_IS(wabi_tag_symbol, e) && WABI_IS(wabi_tag_pair, ctrl)) {
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

wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_env env;
  wabi_symbol sym;
  wabi_val val;

  env = wabi_env_new(vm);

  WABI_DEFX(vm, env, "def", "wabi:def", wabi_builtin_def);
  WABI_DEFX(vm, env, "fx", "wabi:fx", wabi_builtin_fx);
  WABI_DEFN(vm, env, "wrap", "wabi:wrap", wabi_builtin_wrap);
  WABI_DEFN(vm, env, "+", "wabi:+", wabi_builtin_sum);
  WABI_DEFN(vm, env, "-", "wabi:-", wabi_builtin_diff);
  WABI_DEFN(vm, env, "*", "wabi:*", wabi_builtin_mul);
  WABI_DEFN(vm, env, "/", "wabi:/", wabi_builtin_div);
  return env;
}
