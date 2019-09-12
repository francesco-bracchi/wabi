#define wabi_builtin_c

#include "wabi_value.h"
#include "wabi_pair.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_store.h"
#include "wabi_cont.h"
#include "wabi_number.h"
#include "wabi_vm.h"

void
wabi_builtin_sum(wabi_vm vm)
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
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm->store, "sum not a number");
    return;
  }
  vm->control = wabi_fixnum_new(vm->store, res);
}

void
wabi_builtin_fx(wabi_vm vm)
{
  wabi_val ctrl, fs, e, b;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    fs = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      e = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      if(WABI_IS(wabi_tag_env, e) && WABI_IS(wabi_tag_pair, ctrl)) {
        b = wabi_car((wabi_pair) ctrl);
        ctrl = wabi_cdr((wabi_pair) ctrl);
        if(*ctrl == wabi_val_nil) {
          vm->control = (wabi_val) wabi_combiner_new(vm->store, vm->env, e, fs, b);
          return;
        }
      }
    }
    vm->errno = 1;
    vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm->store, "fexpr error");
  }
}

void
wabi_builtin_wrap(wabi_vm vm)
{
  wabi_val ctrl;
  ctrl = vm->control;
  if(WABI_IS(wabi_tag_oper, ctrl) || WABI_IS(wabi_tag_bt_oper, ctrl)) {
    vm->control = (wabi_val) wabi_combiner_wrap(vm->store, (wabi_combiner) ctrl);
  }
  return;
}


void
wabi_builtin_def(wabi_vm vm)
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
        vm->continuation = wabi_cont_def_new(vm->store, vm->env, fs, vm->continuation);
        vm->continuation = wabi_cont_eval_new(vm->store, vm->env, vm->continuation);
        vm->control = e;
        return;
      }
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm->store, "def error");
}


void
wabi_builtin_if(wabi_vm vm)
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
          vm->continuation = wabi_cont_sel_new(vm->store, vm->env, l, r, vm->continuation);
          vm->continuation = wabi_cont_eval_new(vm->store, vm->env, vm->continuation);
          vm->control = t;
          return;
        }
      }
    }
  }
  vm->errno = 1;
  vm->errval = (wabi_val) wabi_binary_leaf_new_from_cstring(vm->store, "if error");
}

#define SYM(store, str)                                                 \
  wabi_symbol_new(store,                                                \
                  (wabi_val) wabi_binary_leaf_new_from_cstring(store, str))

#define BTFUN(store, str, fun)                                          \
  wabi_combiner_builtin_new(store,                            \
                            (wabi_binary) wabi_binary_leaf_new_from_cstring(store, str), \
                            fun)                                        \


#define WABI_DEF(store, env, name, btname, fun) \
  wabi_env_set(store,                                                   \
               env,                                                     \
               SYM(store, name),                                        \
               (wabi_val) BTFUN(store, btname, fun)                     \
               );

wabi_env
wabi_builtin_stdenv(wabi_vm vm)
{
  wabi_env env;
  wabi_symbol sym;
  wabi_val val;

  env = wabi_env_new(vm->store);


  WABI_DEF(vm->store, env, "def", "wabi/def", wabi_builtin_sum);
  WABI_DEF(vm->store, env, "fx", "wabi/fx", wabi_builtin_fx);
  WABI_DEF(vm->store, env, "wrap", "wabi/wrap", wabi_builtin_wrap);
  WABI_DEF(vm->store, env, "+", "wabi/+", wabi_builtin_sum);

  /* wabi_en_set(store, */
  /*             env, */
  /*             wabi_symbol_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "wrap")), */
  /*             wabi_combiner_builtin_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "wabi/wrap"), &wabi_builtin_wrap) */
  /*             ); */

  /* wabi_en_set(store, */
  /*             env, */
  /*             wabi_symbol_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "fx")), */
  /*             wabi_combiner_builtin_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "wabi/fx"), &wabi_builtin_fx) */
  /*             ); */

  /* wabi_en_set(store, */
  /*             env, */
  /*             wabi_symbol_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "+")), */
  /*             wabi_combiner_builtin_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "wabi/+"), &wabi_builtin_sum) */
  /*             ); */

  /* wabi_en_set(store, */
  /*             env, */
  /*             wabi_symbol_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "def")), */
  /*             wabi_combiner_builtin_new(vm->store, wabi_binary_leaf_new_from_cstring(vm->store, "wabi/def"), &wabi_builtin_def) */
  /*             ); */

  return env;
}
