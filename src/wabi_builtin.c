#define wabi_builtin_c

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "wabi_value.h"
#include "wabi_list.h"
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
#include "wabi_atom.h"
#include "wabi_builtin.h"
#include "wabi_delim.h"
#include "wabi_error.h"
#include "wabi_pr.h"
#include "wabi_map.h"
#include "wabi_hash.h"
#include "wabi_symbol.h"
#include "wabi_place.h"
#include "wabi_vector.h"
#include "wabi_vm.h"


static inline void
wabi_builtin_predicate(const wabi_vm vm,
                       const wabi_builtin_test_fn test_fn)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! test_fn(val)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_sum(const wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 0L;
  ctrl = vm->ctrl;


  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if (!wabi_is_fixnum(a)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    ac += WABI_CAST_INT64(a);
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;
  *a = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->ctrl = a;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_mul(const wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 1L;
  ctrl = vm->ctrl;

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if (!wabi_is_fixnum(a)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    ac *= WABI_CAST_INT64(a);
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;
  *a = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
  vm->ctrl = a;
}

static inline void
wabi_builtin_dif(const wabi_vm vm)
{
  long ac;
  wabi_val a, res, ctrl;

  ctrl = vm->ctrl;

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
    if (!wabi_is_fixnum(a)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
  ac = WABI_CAST_INT64(a);

  if(wabi_atom_is_empty(vm, ctrl)) {
    // unary op
    *res = (- ac) & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
    vm->ctrl = res;
    return;
  }

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if (!wabi_is_fixnum(a)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    ac -= WABI_CAST_INT64(a);
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
  }
  *res = ac & wabi_word_value_mask;
  WABI_SET_TAG(res, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
  vm->ctrl = res;
}

static inline void
wabi_builtin_div(const wabi_vm vm)
{
  long x, ac;
  wabi_val a, ctrl, res;

  ctrl = vm->ctrl;

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return;

  a = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if (!wabi_is_fixnum(a)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  ac = WABI_CAST_INT64(a);

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    if (!wabi_is_fixnum(a)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    x = WABI_CAST_INT64(a);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(x == 0) {
      vm->ert = wabi_error_division_by_zero;
      return;
    }
    ac /= x;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  *res = ac & wabi_word_value_mask;
  WABI_SET_TAG(a, wabi_tag_fixnum);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
  vm->ctrl = res;
}

static inline void
wabi_builtin_car(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_car((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  if (wabi_atom_is_empty(vm, pair) || wabi_atom_is_nil(vm, pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}

static inline void
wabi_builtin_cdr(const wabi_vm vm) {
  wabi_val ctrl, pair;
  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  pair = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (wabi_is_pair(pair)) {
    vm->ctrl = (wabi_val)wabi_cdr((wabi_pair)pair);
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  if (wabi_atom_is_empty(vm, pair) || wabi_atom_is_nil(vm, pair)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  vm->ert = wabi_error_type_mismatch;
}

static inline void
wabi_builtin_len(const wabi_vm vm) {
  wabi_val ctrl, list;
  wabi_size l;
  wabi_fixnum r;

  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  list = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  l = wabi_list_length(vm, list);
  if(l < 0) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  r = wabi_fixnum_new(vm, l);
  if(vm->ert) return;
  vm->ctrl = (wabi_val) r;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_cons(const wabi_vm vm)
{
  wabi_val ctrl, a, d, res;

  ctrl = vm->ctrl;
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  a = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_val)wabi_cons(vm, a, d);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

void
wabi_defx(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_word fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner oper;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  oper = wabi_operator_builtin_new(vm, fun);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, (wabi_val) oper);
}

void
wabi_defn(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_word fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner app;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  app = wabi_application_builtin_new(vm, fun);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, (wabi_val) app);
}

void
wabi_def(const wabi_vm vm,
         const wabi_env env,
         char* name,
         wabi_val val)
{
  wabi_binary bin;
  wabi_symbol sym;

  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, val);
}

wabi_env
wabi_builtin_stdenv(const wabi_vm vm)
{
  wabi_env env;

  env = wabi_env_new(vm);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "def", WABI_BT_DEF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "if", WABI_BT_IF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "do", WABI_BT_DO);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pair?", WABI_BT_PAIR_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "num?", WABI_BT_NUM_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "sym?", WABI_BT_SYM_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "atom?", WABI_BT_ATOM_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "bin?", WABI_BT_BIN_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "plc?", WABI_BT_PLC_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "map?", WABI_BT_MAP_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "vec?", WABI_BT_VEC_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "env?", WABI_BT_ENV_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "cont?", WABI_BT_CONT_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "fx?", WABI_BT_FX_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "fn?", WABI_BT_FN_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pr", WABI_BT_PR);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "eval", WABI_BT_EVAL);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "clock", WABI_BT_CLOCK);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "not", WABI_BT_NOT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "hash", WABI_BT_HASH);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "l0", WABI_BT_L0);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "collect", WABI_BT_COLLECT);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "load", WABI_BT_LOAD);
  if(vm->ert) return NULL;

  wabi_defn(vm,env, "cons", WABI_BT_CONS);
  if(vm->ert) return NULL;

  wabi_defn(vm,env, "car", WABI_BT_CAR);
  if(vm->ert) return NULL;

  wabi_defn(vm,env, "cdr", WABI_BT_CDR);
  if(vm->ert) return NULL;

  wabi_defn(vm,env, "list?", WABI_BT_LIST_Q);
  if(vm->ert) return NULL;

  wabi_defn(vm,env, "len", WABI_BT_LEN);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "+", WABI_BT_SUM);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "*", WABI_BT_MUL);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "-", WABI_BT_DIF);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "/", WABI_BT_DIV);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "=", WABI_BT_EQ);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "/=", WABI_BT_NEQ);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, ">", WABI_BT_GT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "<", WABI_BT_LT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, ">=", WABI_BT_GTE);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "<=", WABI_BT_LTE);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "fx", WABI_BT_FX);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "fn", WABI_BT_FN);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "wrap", WABI_BT_WRAP);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "unwrap", WABI_BT_UNWRAP);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "ext", WABI_BT_ENV_EXT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "bin-len", WABI_BT_BIN_LEN);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "bin-cat", WABI_BT_BIN_CAT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "bin-sub", WABI_BT_BIN_SUB);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "prompt", WABI_BT_PROMPT);
  if (vm->ert)return NULL;

  wabi_defx(vm, env, "control", WABI_BT_CONTROL);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "map/new", WABI_BT_MAP_NEW);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "assoc", WABI_BT_ASSOC);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "dissoc", WABI_BT_DISSOC);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "map-len", WABI_BT_MAP_LEN);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "sym", WABI_BT_SYM);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "atom", WABI_BT_ATOM);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "vec", WABI_BT_VEC);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "vec-len", WABI_BT_VEC_LEN);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "push-right", WABI_BT_VEC_PUSH_RIGHT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "push-left", WABI_BT_VEC_PUSH_LEFT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "right", WABI_BT_VEC_RIGHT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "left", WABI_BT_VEC_LEFT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pop-left", WABI_BT_VEC_POP_LEFT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pop-right", WABI_BT_VEC_POP_RIGHT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "vec-concat", WABI_BT_VEC_CONCAT);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "vec-set", WABI_BT_VEC_SET);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "plc", WABI_BT_PLC);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "plc-val", WABI_BT_PLC_VAL);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "plc-cas", WABI_BT_PLC_CAS);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "comb-meta", WABI_BT_CMB_META);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "comb-meta!", WABI_BT_CMB_SET_META);
  if(vm->ert) return NULL;

  return env;
}

static inline void
wabi_builtin_def(const wabi_vm vm)
{
  wabi_val ctrl, lft, rgt, def, es;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env = (wabi_env) ((wabi_cont_call) cont)->env;
  cont = wabi_cont_pop(cont);

  // (def)
  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->cont = (wabi_val) cont;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  lft = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (def lft)
  if (wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_def(vm, env, lft, cont);
    if (vm->ert) return;
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)cont;
    vm->env = (wabi_val)env;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  rgt = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (def lft rgt . ctrl)
  if(!wabi_atom_is_empty(vm, ctrl)) {
    def = (wabi_val) ((wabi_cont_call) vm->cont)->combiner;
    es = (wabi_val) wabi_cons(vm, def, ctrl);
    if(vm->ert) return;
    es = (wabi_val) wabi_cons(vm, es, vm->emp);
    if(vm->ert) return;
    cont = wabi_cont_push_prog(vm, env, es, cont);
    if(vm->ert) return;
  }
  cont = wabi_cont_push_def(vm, env, lft, cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;
  vm->ctrl = rgt;
  vm->cont = (wabi_val) cont;
  vm->env = (wabi_val) env;
}

static inline void
wabi_builtin_if (const wabi_vm vm)
{
  wabi_val ctrl, tst, lft, rgt, slf;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env = (wabi_env) ((wabi_cont_call) cont)->env;
  cont = wabi_cont_pop(cont);

  // (if)
  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) cont;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  tst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (if tst)
  if (wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_eval(vm, cont);
    if (vm->ert) return;
    vm->ctrl = tst;
    vm->cont = (wabi_val)cont;
    vm->env = (wabi_val)env;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // (if tst lft . ctrl)
  lft = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  slf = (wabi_val) ((wabi_cont_call) vm->cont)->combiner;
  rgt = (wabi_val) wabi_cons(vm, slf, ctrl);
  if(vm->ert) return;
  cont = wabi_cont_push_sel(vm, env, lft, rgt, cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = tst;
  vm->cont = (wabi_val) cont;
  vm->env = (wabi_val) env;
}

static inline void
wabi_builtin_eq(const wabi_vm vm)
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
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_neq(const wabi_vm vm)
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
    if(!wabi_cmp(a, b)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_gt(const wabi_vm vm)
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
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_gte(const wabi_vm vm)
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
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_lt(const wabi_vm vm)
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
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_lte(const wabi_vm vm)
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
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
    a = b;
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_pr(const wabi_vm vm)
{
  wabi_val ctrl, v;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    v = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    wabi_pr(vm, v);
    if(WABI_IS(wabi_tag_pair, ctrl)) printf(" ");
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  printf("\n");
  vm->ctrl = vm->nil;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_eval(const wabi_vm vm)
{
  wabi_val ctrl, ex;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  env = (wabi_env) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_env((wabi_val) env)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  ex = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(vm->ert) return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = ex;
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}

static inline void
wabi_builtin_do(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_cont cont;
  wabi_env env;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) cont;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_is_pair(ctrl)) {
    env = (wabi_env) ((wabi_cont_call) vm->cont)->env;
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(vm->ert) return;
  } else if (! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = v;
  vm->cont = (wabi_val) cont;
}

static inline void
wabi_builtin_clock(wabi_vm vm)
{
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_fixnum_new(vm, clock() * 1000000 / CLOCKS_PER_SEC);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);;
}

static inline void
wabi_builtin_not(const wabi_vm vm)
{
  wabi_val v, ctrl;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(vm->ert) return;
  vm->ctrl = wabi_is_falsey(vm, v) ? vm->trh : vm->fls;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_hash(const wabi_vm vm)
{
  wabi_val ctrl, v, r;
  wabi_word hash;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  hash = wabi_hash(v);
  if(! hash) {
    vm->ert = wabi_error_other;
    return;
  }
  r = wabi_fixnum_new(vm, hash);
  if(vm->ert) return;
  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_collect(const wabi_vm vm)
{
  if(!wabi_atom_is_empty(vm, vm->ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  wabi_vm_collect(vm);
  if(vm->ert) return;

  vm->ctrl = vm->nil;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

void
wabi_builtin_language0(wabi_vm vm)
{
  wabi_val ctrl, r;

  ctrl = vm->ctrl;
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  r = (wabi_val) wabi_builtin_stdenv(vm);
  if(vm->ert) return;

  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_load(const wabi_vm vm)
{
  wabi_val ctrl, exs;
  wabi_binary fn;
  char *cfn;
  FILE *f;
  long length;
  char *buffer;
  wabi_cont cont;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fn = (wabi_binary) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_binary((wabi_val) fn)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cfn = wabi_binary_to_cstring(vm, fn);
  if(vm->ert) return;

  f = fopen(cfn, "rb");
  printf("loading %s\n", cfn);
  if(! f)  {
    printf("error reading %s\n", cfn);
    vm->ert = wabi_error_file_not_found;
    return;
  }
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);
  exs = (wabi_val) wabi_reader_read_all(vm, buffer);
  free(buffer);
  if(vm->ert) return;
  cont = (wabi_cont) vm->cont;
  if(wabi_is_pair((wabi_val) exs)) {
    cont = wabi_cont_pop(cont);
    cont = wabi_cont_push_prog(vm, (wabi_env) vm->env, wabi_cdr((wabi_pair) exs), cont);
    if(vm->ert) return;
    cont = wabi_cont_push_eval(vm, cont);
    if(vm->ert) return;
    vm->ctrl = wabi_car((wabi_pair) exs);
    vm->cont = (wabi_val) cont;
    return;
  }

  vm->ctrl = exs;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_prompt(const wabi_vm vm)
{
  wabi_env env;
  wabi_val fst, ctrl, tag;
  wabi_cont cont, prmt;

  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  tag = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  fst = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  env = (wabi_env)((wabi_cont_call)vm->cont)->env;

  cont = wabi_cont_pop((wabi_cont)vm->cont);
  prmt = cont =
      wabi_cont_push_prompt(vm, tag, (wabi_cont_prompt)vm->prmt, cont);
  if (vm->ert)
    return;
  if (wabi_is_pair(ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if (vm->ert)
      return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if (vm->ert)
    return;

  vm->ctrl = fst;
  vm->cont = (wabi_val)cont;
  vm->prmt = (wabi_val)prmt;
}

static inline void
wabi_builtin_control(const wabi_vm vm)
{
  wabi_env env;
  wabi_val kname, ctrl, tag, fst;
  wabi_cont_prompt prompt;
  wabi_cont cont;
  wabi_combiner kval;

  ctrl = vm->ctrl;
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  tag = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  kname = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_symbol(kname) && !wabi_atom_is_ignore(vm, kname)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  fst = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  env = (wabi_env)((wabi_cont_call)vm->cont)->env;
  env = wabi_env_extend(vm, env);
  if (vm->ert)
    return;

  cont = wabi_cont_pop((wabi_cont)vm->cont);
  prompt = (wabi_cont_prompt)vm->prmt;

  kval = wabi_combiner_continuation_new(vm, cont);
  if (vm->ert)
    return;

  for (;;) {
    if (!prompt) {
      vm->ert = wabi_error_no_prompt;
      return;
    }

    if (wabi_eq(tag, (wabi_val)wabi_cont_prompt_tag(prompt)))
      break;

    prompt = wabi_cont_prompt_next_prompt(prompt);
  }
  wabi_env_set(vm, env, kname, (wabi_val)kval);
  if (vm->ert)
    return;

  cont = wabi_cont_pop((wabi_cont)prompt);

  if (wabi_is_pair(ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if (vm->ert)
      return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if (vm->ert)
    return;

  vm->ctrl = fst;
  vm->env = (wabi_val)env;
  vm->cont = (wabi_val)cont;
  vm->prmt = (wabi_val)wabi_cont_prompt_next_prompt(prompt);

  *((wabi_val) prompt) = 0L;
}

static inline void
wabi_builtin_pair_q(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_pair);
}

static inline void
wabi_builtin_list_q(const wabi_vm vm)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_list(vm, val)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_atom_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_atom);
}

static inline void
wabi_builtin_sym_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_symbol);
}

static inline void
wabi_builtin_env_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_env);
}

static inline void
wabi_builtin_map_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_map);
}

static inline void
wabi_builtin_plc_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_place);
}

static inline void
wabi_builtin_num_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_fixnum);
}

static inline void
wabi_builtin_bin_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_binary);
}

static inline void
wabi_builtin_vec_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_vector);
}

static inline void
wabi_builtin_cont_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_combiner_is_continuation);
}

static inline void
wabi_builtin_fx_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_combiner_is_operative);
}

static inline void
wabi_builtin_fn_q(wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_combiner_is_applicative);
}

static inline void
wabi_builtin_wrap(const wabi_vm vm)
{
  wabi_val ctrl, fx, res;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fx = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  switch(WABI_TAG(fx)) {
  case wabi_tag_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(vm->ert) return;
    wordcopy(res, fx, WABI_COMBINER_DERIVED_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_app);
    break;
  case wabi_tag_bt_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(vm->ert) return;
    wordcopy(res, fx, WABI_COMBINER_BUILTIN_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_bt_app);
    break;

  default:
    res = fx;
    break;
  }

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_unwrap(const wabi_vm vm)
{
  wabi_val ctrl, fn, res;
  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fn = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  switch(WABI_TAG(fn)) {
  case wabi_tag_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(vm->ert) return;
    wordcopy(res, fn, WABI_COMBINER_DERIVED_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_oper);
    break;
  case wabi_tag_bt_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(vm->ert) return;
    wordcopy(res, fn, WABI_COMBINER_BUILTIN_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_bt_oper);
    break;

  default:
    res = fn;
    break;
  }

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_fx(const wabi_vm vm)
{
  wabi_combiner_derived fx;
  wabi_val ctrl, e, fs;
  ctrl = vm->ctrl; // (e fs . ctrl)
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  e = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_symbol(e) && !wabi_atom_is_ignore(vm, e)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fs = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fx = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
  if(vm->ert) return;

  fx->static_env = (wabi_word) ((wabi_cont_call) vm->cont)->env;
  fx->caller_env_name = (wabi_word) e;
  fx->parameters = (wabi_word) fs;
  fx->body = (wabi_word) ctrl;
  fx->meta = (wabi_word) vm->nil;
  WABI_SET_TAG(fx, wabi_tag_oper);
  vm->ctrl = (wabi_val) fx;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_fn(const wabi_vm vm)
{
  wabi_combiner_derived fn;
  wabi_val ctrl, fs;
  ctrl = vm->ctrl; // (fs . ctrl)
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fs = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fn = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
  if(vm->ert) return;

  fn->static_env = (wabi_word) ((wabi_cont_call) vm->cont)->env;
  fn->caller_env_name = (wabi_word) vm->ign;
  fn->parameters = (wabi_word) fs;
  fn->body = (wabi_word) ctrl;
  fn->meta = (wabi_word) vm->nil;
  WABI_SET_TAG(fn, wabi_tag_app);
  vm->ctrl = (wabi_val) fn;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_env_ext(const wabi_vm vm)
{
  // todo: how about multiple extensions?
  wabi_val ctrl, res;
  wabi_env e0;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  e0 = (wabi_env) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_env((wabi_val) e0)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  res = (wabi_val) wabi_env_extend(vm, e0);
  if(vm->ert) return;

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_bin_len(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_binary bin;
  wabi_size sum;
  wabi_fixnum res;

  ctrl = vm->ctrl;
  sum = 0L;

  while(wabi_is_pair(ctrl)) {
    bin = (wabi_binary) wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(!wabi_is_binary((wabi_val) bin)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }

    sum += wabi_binary_length(bin);
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  res = wabi_fixnum_new(vm, sum);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_bin_cat(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_binary bin0, bin;

  ctrl = vm->ctrl;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, "");
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    bin0 = (wabi_binary) wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(!wabi_is_binary((wabi_val) bin0)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    if(wabi_binary_length(bin0)) {
      bin = (wabi_binary) wabi_binary_node_new(vm, bin, bin0);
    }
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = (wabi_val) bin;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_bin_sub(const wabi_vm vm)
{
  wabi_binary bin;
  wabi_fixnum offset, len;
  wabi_val ctrl;
  wabi_word os, l, l0;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  bin = (wabi_binary) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_binary((wabi_val) bin)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  offset = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_fixnum((wabi_val) offset)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  len = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_fixnum((wabi_val) len)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  os = WABI_CAST_INT64(offset);
  l = WABI_CAST_INT64(len);
  l0 = wabi_binary_length(bin);

  if(os < 0 || os >= l0 || l < 0 || l >= l0 - os) {
    vm->ert = wabi_error_out_of_range;
    return;
  }

  bin = wabi_binary_sub(vm, bin, os, l);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) bin;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_map_new(const wabi_vm vm)
{
  // if arity is odd it raises an error.
  // the alternative is that is associate the last value (key) to
  // nil, but it's an inconsistent behavior, i.e.
  // `((map/new "a" 10) "b")` -> `nil`
  //  ((map/new "a") "a") -> `nil`
  wabi_val ctrl, k, v;
  wabi_map res;
  ctrl = vm->ctrl;
  res = wabi_map_empty(vm);
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_pair(ctrl)) {
      vm->ert = wabi_error_bindings;
      return;
    }
    v = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_map_assoc(vm, res, k, v);
    if(vm->ert) return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static inline void
wabi_builtin_assoc(const wabi_vm vm)
{
  wabi_val ctrl, k, v;
  wabi_map res;
  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_map) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_map((wabi_val) res)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(wabi_is_pair(ctrl)) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_pair(ctrl)) {
      vm->ert = wabi_error_bindings;
      return;
    }
    v = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_map_assoc(vm, res, k, v);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_dissoc(const wabi_vm vm)
{
  wabi_val ctrl, k, res;
  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_car((wabi_pair) ctrl);
  ctrl  = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_map(res)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = (wabi_val) wabi_map_dissoc(vm, (wabi_map) res, k);
    if(vm->ert) return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_map_len(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_map m;
  wabi_fixnum len;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  m = (wabi_map) wabi_car((wabi_pair) ctrl);
  ctrl  = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_map((wabi_val) m)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  len = wabi_fixnum_new(vm, wabi_map_length(m));
  if(vm->ert) return;

  vm->ctrl = len;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_sym(const wabi_vm vm)
{
  wabi_val ctrl, bin, res;

  ctrl = vm->ctrl;

  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  bin = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_atom_is_nil(vm, bin)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  if (!wabi_is_binary(bin)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  res = wabi_symbol_new(vm, bin);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_atom(const wabi_vm vm)
{
  wabi_val ctrl, bin, res;

  ctrl = vm->ctrl;

  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  bin = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_atom_is_nil(vm, bin)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  if (!wabi_is_binary(bin)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  res = wabi_atom_new(vm, bin);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static inline void
wabi_builtin_vec(const wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  res = (wabi_vector) wabi_vector_digit_new(vm, 0, 0);
  if(vm->ert) return;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_concat(const wabi_vm vm)
{
  wabi_val ctrl, d;
  wabi_vector r;
  ctrl = vm->ctrl;
  r = (wabi_vector) wabi_vector_digit_new(vm, 0, 0);
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    d = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_vector(d)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    r = wabi_vector_concat(vm, r, (wabi_vector) d);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  vm->ctrl = (wabi_val) r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_len(const wabi_vm vm)
{
  wabi_vector d;
  wabi_val r, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  r = (wabi_val) wabi_fixnum_new(vm, wabi_vector_size(d));
  if(vm->ert) return;

  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_push_right(const wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) res)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_push_left(const wabi_vm vm)
{
  wabi_vector d;
  wabi_val ctrl, v;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = wabi_vector_push_left(vm, v, d);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_left(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = wabi_vector_left(vm, d);
  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_right(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = wabi_vector_right(vm, d);

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_pop_left(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = (wabi_val) wabi_vector_pop_left(vm, d);
  if(!v) v = vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_pop_right(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = (wabi_val) wabi_vector_pop_right(vm, d);
  if(! v) v = vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_ref(const wabi_vm vm)
{
  wabi_val res, ctrl;
  wabi_vector d;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair((wabi_val) ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  n = (wabi_fixnum) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_fixnum(n)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  x = WABI_CAST_INT64(n);
  s = wabi_vector_size(d);

  if(x < 0 || x >= s) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
    return;
  }
  res = wabi_vector_ref(d, x);
  if(! res) {
    vm->ert = wabi_error_other;
    return;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_vec_set(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if (!wabi_is_vector((wabi_val)d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(wabi_is_pair(ctrl)) {

    n = (wabi_fixnum)wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    if (!wabi_is_fixnum(n)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    if (!wabi_is_pair(ctrl)) {
      vm->ert = wabi_error_bindings;
      return;
    }
    v = wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    x = WABI_CAST_INT64(n);
    s = wabi_vector_size(d);

    if (x < 0 || x >= s) {
      vm->ctrl = vm->nil;
      vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
      return;
    }
    d = wabi_vector_set(vm, d, x, v);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_plc(const wabi_vm vm)
{
  wabi_val ctrl, init;
  wabi_place res;

  ctrl = vm->ctrl;
  init = vm->nil;

  if(wabi_is_pair(ctrl)) {
    init = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_place_new(vm, init);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_plc_val(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(!wabi_is_place((wabi_val) plc)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = wabi_place_val(plc);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static inline void
wabi_builtin_plc_cas(const wabi_vm vm)
{
  wabi_val ctrl, res;
  wabi_word old, new;
  wabi_place plc;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  plc = (wabi_place) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_place((wabi_val) plc)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  old = (wabi_word) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  new = (wabi_word) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }
  res = __sync_bool_compare_and_swap (&plc->val, old, new) ? vm->trh : vm->fls;

  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}


void
wabi_builtin_combiner_meta(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_combiner_derived comb;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  comb = (wabi_combiner_derived) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }

  if(wabi_atom_is_nil(vm, (wabi_val) comb)){
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }

  if(! wabi_combiner_is_derived((wabi_val) comb)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  vm->ctrl = (wabi_val) wabi_combiner_derived_meta(comb);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont)vm->cont);
}


void
wabi_builtin_combiner_set_meta(const wabi_vm vm)
{
  wabi_val ctrl, meta;
  wabi_combiner_derived comb;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  comb = (wabi_combiner_derived) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_combiner_is_derived((wabi_val) comb)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  meta = (wabi_val) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }

  comb->meta = (wabi_word) meta;
  vm->ctrl = meta;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont)vm->cont);
}


void
wabi_builtin_call(const wabi_vm vm,
                  const wabi_word func)
{
  switch (func) {
  case WABI_BT_CONS:
    wabi_builtin_cons(vm);
    break;
  case WABI_BT_CAR:
    wabi_builtin_car(vm);
    break;
  case WABI_BT_CDR:
    wabi_builtin_cdr(vm);
    break;
  case WABI_BT_DEF:
    wabi_builtin_def(vm);
    break;
  case WABI_BT_IF:
    wabi_builtin_if(vm);
    break;
  case WABI_BT_DO:
    wabi_builtin_do(vm);
    break;
  case WABI_BT_EVAL:
    wabi_builtin_eval(vm);
    break;
  case WABI_BT_PAIR_Q:
    wabi_builtin_pair_q(vm);
    break;
  case WABI_BT_NUM_Q:
    wabi_builtin_num_q(vm);
    break;
  case WABI_BT_SYM_Q:
    wabi_builtin_sym_q(vm);
    break;
  case WABI_BT_ATOM_Q:
    wabi_builtin_atom_q(vm);
    break;
  case WABI_BT_BIN_Q:
    wabi_builtin_bin_q(vm);
    break;
  case WABI_BT_PLC_Q:
    wabi_builtin_plc_q(vm);
    break;
  case WABI_BT_MAP_Q:
    wabi_builtin_map_q(vm);
    break;
  case WABI_BT_VEC_Q:
    wabi_builtin_vec_q(vm);
    break;
  case WABI_BT_ENV_Q:
    wabi_builtin_env_q(vm);
    break;
  case WABI_BT_CONT_Q:
    wabi_builtin_cont_q(vm);
    break;
  case WABI_BT_FX_Q:
    wabi_builtin_fx_q(vm);
    break;
  case WABI_BT_FN_Q:
    wabi_builtin_fn_q(vm);
    break;
  case WABI_BT_SUM:
    wabi_builtin_sum(vm);
    break;
  case WABI_BT_MUL:
    wabi_builtin_mul(vm);
    break;
  case WABI_BT_DIF:
    wabi_builtin_dif(vm);
    break;
  case WABI_BT_DIV:
    wabi_builtin_div(vm);
    break;
  case WABI_BT_EQ:
    wabi_builtin_eq(vm);
    break;
  case WABI_BT_NEQ:
    wabi_builtin_neq(vm);
    break;
  case WABI_BT_GT:
    wabi_builtin_gt(vm);
    break;
  case WABI_BT_LT:
    wabi_builtin_lt(vm);
    break;
  case WABI_BT_GTE:
    wabi_builtin_gte(vm);
    break;
  case WABI_BT_LTE:
    wabi_builtin_lte(vm);
    break;
  case WABI_BT_PROMPT:
    wabi_builtin_prompt(vm);
    break;
  case WABI_BT_CONTROL:
    wabi_builtin_control(vm);
    break;
  case WABI_BT_MAP_NEW:
    wabi_builtin_map_new(vm);
    break;
  case WABI_BT_ASSOC:
    wabi_builtin_assoc(vm);
    break;
  case WABI_BT_DISSOC:
    wabi_builtin_dissoc(vm);
    break;
  case WABI_BT_MAP_LEN:
    wabi_builtin_map_len(vm);
    break;
  case WABI_BT_SYM:
    wabi_builtin_sym(vm);
    break;
  case WABI_BT_ATOM:
    wabi_builtin_atom(vm);
    break;
  case WABI_BT_PLC:
    wabi_builtin_plc(vm);
    break;
  case WABI_BT_PLC_VAL:
    wabi_builtin_plc_val(vm);
    break;
  case WABI_BT_PLC_CAS:
    wabi_builtin_plc_cas(vm);
    break;
  case WABI_BT_VEC:
    wabi_builtin_vec(vm);
    break;
  case WABI_BT_VEC_PUSH_LEFT:
    wabi_builtin_vec_push_left(vm);
    break;
  case WABI_BT_VEC_PUSH_RIGHT:
    wabi_builtin_vec_push_right(vm);
    break;
  case WABI_BT_VEC_LEFT:
    wabi_builtin_vec_left(vm);
    break;
  case WABI_BT_VEC_RIGHT:
    wabi_builtin_vec_right(vm);
    break;
  case WABI_BT_VEC_POP_LEFT:
    wabi_builtin_vec_pop_left(vm);
    break;
  case WABI_BT_VEC_POP_RIGHT:
    wabi_builtin_vec_pop_right(vm);
    break;
  case WABI_BT_VEC_LEN:
    wabi_builtin_vec_len(vm);
    break;
  case WABI_BT_VEC_CONCAT:
    wabi_builtin_vec_concat(vm);
    break;
  case WABI_BT_VEC_SET:
    wabi_builtin_vec_set(vm);
    break;
  case WABI_BT_FX:
    wabi_builtin_fx(vm);
    break;
  case WABI_BT_FN:
    wabi_builtin_fn(vm);
    break;
  case WABI_BT_WRAP:
    wabi_builtin_wrap(vm);
    break;
  case WABI_BT_UNWRAP:
    wabi_builtin_unwrap(vm);
    break;
  case WABI_BT_LIST_Q:
    wabi_builtin_list_q(vm);
    break;
  case WABI_BT_NOT:
    wabi_builtin_not(vm);
    break;
  case WABI_BT_CLOCK:
    wabi_builtin_clock(vm);
    break;
  case WABI_BT_PR:
    wabi_builtin_pr(vm);
    break;
  case WABI_BT_L0:
    wabi_builtin_language0(vm);
    break;
  case WABI_BT_LOAD:
    wabi_builtin_load(vm);
    break;
  case WABI_BT_LEN:
    wabi_builtin_len(vm);
    break;
  case WABI_BT_ENV_EXT:
    wabi_builtin_env_ext(vm);
    break;
  case WABI_BT_HASH:
    wabi_builtin_hash(vm);
    break;
  case WABI_BT_BIN_LEN:
    wabi_builtin_bin_len(vm);
    break;
  case WABI_BT_BIN_CAT:
    wabi_builtin_bin_cat(vm);
    break;
  case WABI_BT_BIN_SUB:
    wabi_builtin_bin_sub(vm);
    break;
  case WABI_BT_COLLECT:
    wabi_builtin_collect(vm);
    break;
  case WABI_BT_CMB_META:
    wabi_builtin_combiner_meta(vm);
    break;
  case WABI_BT_CMB_SET_META:
    wabi_builtin_combiner_set_meta(vm);
    break;
  default:
    vm->ert = wabi_error_other;
    break;
  }
}
