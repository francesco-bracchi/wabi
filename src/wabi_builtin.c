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
wabi_builtin_sum(const wabi_vm vm)
{
  long ac;
  wabi_val a, ctrl;

  ac = 0L;
  ctrl = vm->ctrl;


  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
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
  ac = WABI_CAST_INT64(a);

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
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
wabi_builtin_number_env(const wabi_vm vm,
                        const wabi_env env)
{
  wabi_defn(vm, env, "+", (wabi_builtin_fun) WABI_BT_SUM);
  if(vm->ert) return;
  wabi_defn(vm, env, "*", (wabi_builtin_fun) WABI_BT_MUL);
  if(vm->ert) return;
  wabi_defn(vm, env, "-", (wabi_builtin_fun) WABI_BT_DIF);
  if(vm->ert) return;
  wabi_defn(vm, env, "/", (wabi_builtin_fun) WABI_BT_DIV);
  if(vm->ert) return;
}


// TODO
// rename with eq gt lt since = < > must be for numbers only (maybe not =?)
void
wabi_builtin_cmp_env(const wabi_vm vm,
                     const wabi_env env)
{
  wabi_defn(vm, env, "=", (wabi_builtin_fun) WABI_BT_EQ);
  if(vm->ert) return;
  wabi_defn(vm, env, "/=", (wabi_builtin_fun) WABI_BT_NEQ);
  if(vm->ert) return;
  wabi_defn(vm, env, ">", (wabi_builtin_fun) WABI_BT_GT);
  if(vm->ert) return;
  wabi_defn(vm, env, "<", (wabi_builtin_fun) WABI_BT_LT);
  if(vm->ert) return;
  wabi_defn(vm, env, ">=", (wabi_builtin_fun) WABI_BT_GTE);
  if(vm->ert) return;
  wabi_defn(vm, env, "<=", (wabi_builtin_fun) WABI_BT_LTE);
}

void
wabi_builtin_list_env(const wabi_vm vm,
                      const wabi_env env)
{
  wabi_defn(vm,env, "cons", (wabi_builtin_fun) WABI_BT_CONS);
  if(vm->ert) return;
  wabi_defn(vm,env, "car", (wabi_builtin_fun) WABI_BT_CAR);
  if(vm->ert) return;
  wabi_defn(vm,env, "cdr", (wabi_builtin_fun) WABI_BT_CDR);
  if(vm->ert) return;
  wabi_defn(vm,env, "pair?", (wabi_builtin_fun) WABI_BT_PAIR_Q);
  if(vm->ert) return;
  wabi_defn(vm,env, "list?", (wabi_builtin_fun) WABI_BT_LIST_Q);
  if(vm->ert) return;
  wabi_defn(vm,env, "len", (wabi_builtin_fun) WABI_BT_LEN);
  if(vm->ert) return;
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

void
wabi_defx(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_builtin_fun fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner oper;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  oper = wabi_operator_builtin_new(vm, bin, fun);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, (wabi_val) oper);
}


void
wabi_defn(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_builtin_fun fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner app;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  app = wabi_application_builtin_new(vm, bin, fun);
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


// todo: move in wabi_builtin.h
void
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

void
wabi_builtin_load_cstring(const wabi_vm vm, const wabi_env env, char* str)
{
  wabi_pair exs;
  wabi_cont cont;

  exs = (wabi_pair) wabi_reader_read_all(vm, str);
  if(vm->ert) return;
  if(! wabi_is_pair((wabi_val) exs)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cont = (wabi_cont) vm->cont;
  if(cont) cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_prog(vm, env, wabi_cdr(exs), cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = wabi_car(exs);
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
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


static void
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



static void
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

wabi_env
wabi_builtin_stdenv(const wabi_vm vm)
{
  wabi_env env;

  env = wabi_env_new(vm);

  if(vm->ert) return NULL;

  wabi_defx(vm, env, "def", (wabi_builtin_fun) WABI_BT_DEF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "if", (wabi_builtin_fun) WABI_BT_IF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "do", wabi_builtin_do);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pr", wabi_builtin_pr);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "eval", wabi_builtin_eval);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "clock", wabi_builtin_clock);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "not", wabi_builtin_not);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "hash", wabi_builtin_hash);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "l0", wabi_builtin_language0);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "collect", wabi_builtin_collect);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "load", &wabi_builtin_load);
  if(vm->ert) return NULL;

  wabi_combiner_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_builtin_list_env(vm, env);
  if(vm->ert) return NULL;

  wabi_builtin_number_env(vm, env);
  if(vm->ert) return NULL;

  wabi_binary_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_builtin_cmp_env(vm, env);
  if(vm->ert) return NULL;

  wabi_env_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_delim_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_map_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_symbol_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_place_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_vector_builtins(vm, env);
  if(vm->ert) return NULL;

  return env;
}

void
wabi_builtin_call(wabi_vm vm,
                  wabi_word func)
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
  /* case WABI_BT_NUM_Q: */
  /*   wabi_builtin_num_q(vm); */
  /*   break; */
  /* case WABI_BT_SYM_Q: */
  /*   wabi_builtin_sym_q(vm); */
  /*   break; */
  /* case WABI_BT_ATOM_Q: */
  /*   wabi_builtin_atom_q(vm); */
  /*   break; */
  /* case WABI_BT_BIN_Q: */
  /*   wabi_builtin_bin(vm); */
  /*   break; */
  /* case WABI_BT_PLC_Q: */
  /*   wabi_builtin_plc_q(vm); */
  /*   break; */
  /* case WABI_BT_MAP_Q: */
  /*   wabi_builtin_map_q(vm); */
  /*   break; */
  /* case WABI_BT_VEC_Q: */
  /*   wabi_builtin_vec_q(vm); */
  /*   break; */
  /* case WABI_BT_ENV_Q: */
  /*   wabi_builtin_env_q(vm); */
  /*   break; */
  /* case WABI_BT_CONT_Q: */
  /*   wabi_builtin_cont_q(vm); */
  /*   break; */
  /* case WABI_BT_FX_Q: */
  /*   wabi_builtin_fx_q(vm); */
  /*   break; */
  /* case WABI_BT_FN_Q: */
  /*   wabi_builtin_fn_q(vm); */
  /*   break; */
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
  /* case WABI_BT_PROMPT: */
  /*   wabi_builtin_prompt(vm); */
  /*   break; */
  /* case WABI_BT_CONTROL: */
  /*   wabi_builtin_control(vm); */
  /*   break; */
  /* case WABI_BT_HMAP: */
  /*   wabi_builtin_map_new(vm); */
  /*   break; */
  /* case WABI_BT_ASSOC: */
  /*   wabi_builtin_assoc(vm); */
  /*   break; */
  /* case WABI_BT_DISSOC: */
  /*   wabi_builtin_dissoc(vm); */
  /*   break; */
  /* case WABI_BT_MAP_LEN: */
  /*   wabi_builtin_map_len(vm); */
  /*   break; */
  /* case WABI_BT_MAP_GET: */
  /*   wabi_builtin_map_get(vm); */
  /*   break; */
  /* case WABI_BT_SYM: */
  /*   wabi_builtin_sym(vm); */
  /*   break; */
  /* case WABI_BT_ATOM: */
  /*   wabi_builtin_atom(vm); */
  /*   break; */
  /* case WABI_BT_PLC: */
  /*   wabi_builtin_plc(vm); */
  /*   break; */
  /* case WABI_BT_PLC_VAL: */
  /*   wabi_builtin_plc_val(vm); */
  /*   break; */
  /* case WABI_BT_PLC_CAS: */
  /*   wabi_builtin_plc_cas(vm); */
  /*   break; */
  /* case WABI_BT_VEC: */
  /*   wabi_builtin_vec(vm); */
  /*   break; */
  /* case WABI_BT_VEC_PUSH_LEFT: */
  /*   wabi_builtin_vec_push_left(vm); */
  /*   break; */
  /* case WABI_BT_VEC_PUSH_RIGHT: */
  /*   wabi_builtin_vec_push_right(vm); */
  /*   break; */
  /* case WABI_BT_VEC_LEFT: */
  /*   wabi_builtin_vec_left(vm); */
  /*   break; */
  /* case WABI_BT_VEC_RIGHT: */
  /*   wabi_builtin_vec_right(vm); */
  /*   break; */
  /* case WABI_BT_VEC_POP_LEFT: */
  /*   wabi_builtin_vec_pop_left(vm); */
  /*   break; */
  /* case WABI_BT_VEC_POP_RIGHT: */
  /*   wabi_builtin_vec_pop_right(vm); */
  /*   break; */
  /* case WABI_BT_VEC_LEN: */
  /*   wabi_builtin_vec_len(vm); */
  /*   break; */
  /* case WABI_BT_VEC_CONCAT: */
  /*   wabi_builtin_vec_concat(vm); */
  /*   break; */
  /* case WABI_BT_VEC_REF: */
  /*   wabi_builtin_vec_ref(vm); */
  /*   break; */
  /* case WABI_BT_VEC_SET: */
  /*   wabi_builtin_vec_set(vm); */
  /*   break; */
  /* case WABI_BT_FX: */
  /*   wabi_builtin_fx(vm); */
  /*   break; */
  /* case WABI_BT_FN: */
  /*   wabi_builtin_fn(vm); */
  /*   break; */
  /* case WABI_BT_WRAP: */
  /*   wabi_builtin_wrap(vm); */
  /*   break; */
  /* case WABI_BT_UNWRAP: */
  /*   wabi_builtin_unwrap(vm); */
  /*   break; */
  case WABI_BT_LIST_Q:
    wabi_builtin_list_q(vm);
  /*   break; */
  /* case WABI_BT_NOT: */
  /*   wabi_builtin_not(vm); */
  /*   break; */
  /* case WABI_BT_CLOCK: */
  /*   wabi_builtin_clock(vm); */
  /*   break; */
  /* case WABI_BT_PR: */
  /*   wabi_builtin_pr(vm); */
  /*   break; */
  /* case WABI_BT_L0: */
  /*   wabi_builtin_l0(vm); */
  /*   break; */
  /* case WABI_BT_LOAD: */
  /*   wabi_builtin_load(vm); */
  /*   break; */
  case WABI_BT_LEN:
    wabi_builtin_len(vm);
    break;
  /* case WABI_BT_ENV_EXT: */
  /*   wabi_builtin_env_ext(vm); */
  /*   break; */
  default:
    ((wabi_builtin_fun)func)(vm);
    break;
  }
}
