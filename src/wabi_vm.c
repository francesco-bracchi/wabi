/**
 * Here we have a `CESK` like VM where CESK is an acronym for
 * `Control` `Environment` `Store` `Kontinuation` VM.
 *
 * ## Expression Evaluation
 *
 * 1. Evaluation for whatever is not a symbol or a consed pair is
 *    the identity function, and evaluation stops.
 *
 * 2. for symbols the value is looked up in the environment.
 *
 * for pairs, (that's the interesting part) the car part is evaluated.

 * 3. If the result is an operator (i.e. something
 * that applies to the code, like `if`...) it is called with argument
 * the cdr part of the pair.
 *
 * 4. Otherwise it's an applicative (i.e. a function) so
 * all the cdr part is evaluated, and the resulting value is passed
 * as argument to the car part.
 *
 * That's it. just 4 cases.
 *
 * ## Partial continuations
 *
 * a partial continuation is a slice of the continuation stack.
 * Applying it is a matter of concatenating the partial continuation continuation slice
 * on top of the current continuation.
 *
 * To build a continuation use the `prompt` and `control` operators.
 * For a full description of partial continuations see ...TBD...
 *
 * ## Reduction steps
 *
 * Despite the simple evaluation expression algotithm, some more work is needed
 * To handle continuations, expression sequences, argument evaluation, combiner
 * application ect...
 *
 * see `vm.txt` for all the steps this VM goes through.
 */
#define wabi_vm_c

#include <stdio.h>
#include "wabi_vm.h"
#include "wabi_store.h"
#include "wabi_system.h"
#include "wabi_collect.h"
#include "wabi_cont.h"
#include "wabi_list.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_binary.h"
#include "wabi_symbol.h"
#include "wabi_atom.h"
#include "wabi_map.h"
#include "wabi_vector.h"
#include "wabi_error.h"
#include "wabi_pr.h"
#include "wabi_builtin.h"
#include "wabi_number.h"
#include "wabi_cmp.h"

wabi_val
wabi_vm_atom_from_cstring(const wabi_vm vm, const char *cstring) {
  wabi_val bin;
  bin = (wabi_val)wabi_binary_leaf_new_from_cstring(vm, cstring);
  if(vm->ert) return NULL;

  return (wabi_val) wabi_atom_new(vm, bin);
}


void
wabi_vm_init(const wabi_vm vm, const wabi_size size)
{
  wabi_val atbl, stbl, emp, nil, ign, fls, trh;

  if(wabi_store_init(&(vm->stor), size)) {
    vm->ert = wabi_error_nomem;
    return;
  }
  stbl = (wabi_val) wabi_map_empty(vm);
  if(vm->ert) return;

  atbl = (wabi_val) wabi_map_empty(vm);
  if(vm->ert) return;

  vm->ert = wabi_error_none;
  vm->stbl  = stbl;
  vm->atbl  = atbl;

  emp = wabi_vm_atom_from_cstring(vm, "()");
  if(vm->ert) return;

  nil = wabi_vm_atom_from_cstring(vm, "nil");
  if(vm->ert) return;

  ign = wabi_vm_atom_from_cstring(vm, "_");
  if(vm->ert) return;

  fls = wabi_vm_atom_from_cstring(vm, "f");
  if(vm->ert) return;

  trh = wabi_vm_atom_from_cstring(vm, "t");
  if(vm->ert) return;

  vm->ctrl = NULL;
  vm->env = NULL;
  vm->cont = (wabi_val) wabi_cont_done;
  vm->prmt = (wabi_val) wabi_cont_done;
  vm->emp = emp;
  vm->nil = nil;
  vm->ign = ign;
  vm->fls = fls;
  vm->trh = trh;
  vm->fuel = 0;
  vm->erv = nil;
  vm->ert = wabi_error_none;
}


void
wabi_vm_collect(const wabi_vm vm)
{
  vm->ert = wabi_error_none;
  wabi_store_prepare(&vm->stor);
  // todo: allocate before collecting
  vm->stbl = (wabi_val) wabi_map_empty(vm);
  vm->atbl = (wabi_val) wabi_map_empty(vm);
  if(vm->ert) return;
  vm->stor.scan += 2 * WABI_MAP_SIZE;

  if(vm->ctrl) vm->ctrl = wabi_copy_val(vm, vm->ctrl);
  if(vm->ert) return;
  if(vm->env) vm->env = wabi_copy_val(vm, vm->env);
  if(vm->ert) return;
  if(vm->cont) vm->cont = wabi_copy_val(vm, vm->cont);
  if(vm->ert) return;
  if(vm->prmt) vm->prmt = wabi_copy_val(vm, vm->prmt);
  if(vm->ert) return;
  if(vm->stbl) vm->stbl = wabi_copy_val(vm, vm->stbl);
  if(vm->ert) return;
  if(vm->emp) vm->emp = wabi_copy_val(vm, vm->emp);
  if(vm->ert) return;
  if(vm->nil) vm->nil = wabi_copy_val(vm, vm->nil);
  if(vm->ert) return;
  if(vm->nil) vm->ign = wabi_copy_val(vm, vm->ign);
  if(vm->ert) return;
  if(vm->fls) vm->fls = wabi_copy_val(vm, vm->fls);
  if(vm->ert) return;
  if(vm->trh) vm->trh = wabi_copy_val(vm, vm->trh);
  if(vm->ert) return;

  wabi_collect(vm);
}


static inline int
wabi_vm_free_words(wabi_vm vm)
{
  return wabi_store_free_words(&vm->stor);
}


void
wabi_vm_destroy(wabi_vm vm)
{
  wabi_store_destroy(&vm->stor);
}


static inline wabi_val
wabi_vm_reverse(const wabi_vm vm,
                const wabi_val done0,
                const wabi_val res0)
{
  wabi_val res;
  wabi_val done;
  done = done0;
  res = res0;
  while(wabi_is_pair(done)) {
    res = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) done), res);
    if(vm->ert) return NULL;
    done = wabi_cdr((wabi_pair) done);
  }
  return res;
}


inline static void
wabi_vm_bind(const wabi_vm vm,
             const wabi_env env,
             wabi_val args,
             wabi_val params)
{
  wabi_val a0, p0;

  for(;;) {
    if(wabi_is_pair(params)) {
      p0 = wabi_car((wabi_pair) params);
      params = wabi_cdr((wabi_pair) params);
      if(wabi_is_pair(args)) {
        a0 = wabi_car((wabi_pair) args);
        args = wabi_cdr((wabi_pair) args);
        wabi_vm_bind(vm, env, a0, p0);
        if(vm->ert) return;
        continue;
      }
      vm->ert = wabi_error_bindings;
      return;
      /* if(wabi_atom_is_empty(vm, args)) { */
      /*   wabi_vm_bind(vm, env, vm->emp, p0); */
      /*   if(vm->ert) return; */
      /*   continue; */
      /* } */
    }
    if(wabi_is_symbol(params)) {
      wabi_env_def(vm, env, (wabi_symbol) params, args);
      return;
    }
    if(wabi_atom_is_ignore(vm, params))
      return;

    if(wabi_cmp(params, args))
      vm->ert = wabi_error_bindings;

    return;
  }
}


static inline wabi_val
wabi_vm_reverse_concat(const wabi_vm vm,
                       wabi_val done,
                       wabi_val res)
{
  while(wabi_is_pair(done)) {
    res = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) done), res);
    if(vm->ert) return NULL;
    done = wabi_cdr((wabi_pair) done);
  }
  return res;
}


/* Combination application */
/* ctrl: (f . as) */
/* envr: e */
/* cont: ((eval) . s) */
/*       -------------------------------------- */
/* ctrl: f */
/* envr: e */
/* cont: ((eval) (apply e0 as) . s) */
static inline void
wabi_vm_reduce_eval_combination(const wabi_vm vm)
{
  wabi_pair expr;
  wabi_val comb, args;
  wabi_cont cont;
  wabi_env env;

  expr = (wabi_pair) vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env  = (wabi_env) vm->env;

  comb = wabi_car(expr);
  args = wabi_cdr(expr);

  cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_apply(vm, env, args, cont);
  if(vm->ert) return;

  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) comb;
  vm->cont = (wabi_val) cont;
}


/* ctrl: c when (sym? c) */
/* envr: e */
/* cont: ((eval) . s) */
/* -------------------------------------- */
/* ctrl: (lookup c e) */
/* envr: e */
/* cont: s */
static inline void
wabi_vm_reduce_eval_symbol(const wabi_vm vm)
{
  wabi_symbol symb;
  wabi_val val;
  wabi_cont cont;
  wabi_env env;

  symb = (wabi_symbol) vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env  = (wabi_env) vm->env;

  val = wabi_env_lookup(env, symb);
  if (!val) {
    vm->ert = wabi_error_unbound_name;
    return;
  }

  cont = wabi_cont_pop(cont);
  vm->ctrl = val;
  vm->cont = (wabi_val) cont;
}


/* ctrl: c */
/* envr: e */
/* cont: ((eval) . s) */
/* -------------------------------------- */
/* ctrl: c */
/* envr: e */
/* cont: s */
static inline void
wabi_vm_reduce_eval_self(const wabi_vm vm)
{
  wabi_cont cont;

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);
  vm->cont = (wabi_val) cont;
}

static inline void
wabi_vm_reduce_eval(const wabi_vm vm)
{
  switch (WABI_TAG(vm->ctrl)) {
  case wabi_tag_pair:
    wabi_vm_reduce_eval_combination(vm);
    break;
  case wabi_tag_symbol:
    wabi_vm_reduce_eval_symbol(vm);
    break;
  default:
    wabi_vm_reduce_eval_self(vm);
    break;
  }
}


/* ctrl: c */
/* envr: e */
/* cont: ((prompt p) . s) */
/* prmt: p0 */
/* -------------------------------------- */
/* ctrl: c */
/* envr: e */
/* cont: s */
/* prmt: p */
static inline void
wabi_vm_reduce_prompt(const wabi_vm vm)
{
  wabi_cont cont;
  wabi_cont_prompt prmt;

  cont = (wabi_cont) vm->cont;
  prmt = (wabi_cont_prompt) vm->prmt;

  cont = wabi_cont_pop(cont);
  prmt = wabi_cont_prompt_next_prompt(prmt);

  vm->cont = (wabi_val) cont;
  vm->prmt = (wabi_val) prmt;
}


/* ctrl: c when (oper? c) */
/* envr: e */
/* cont: ((apply e0 as) . s) */
/* -------------------------------------- */
/* ctrl: as */
/* envr: e0 */
/* cont: ((call e0 c) . s) */
static inline void
wabi_vm_reduce_apply_operative(const wabi_vm vm,
                               const wabi_val comb,
                               const wabi_val env,
                               const wabi_val args)
{
  wabi_cont cont;

  cont = (wabi_cont) vm->cont;

  cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_call(vm, (wabi_env) env, comb, cont);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) args;
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}


/* ctrl: c when (app? c) */
/* envr: e */
/* cont: ((apply e0 (a . as)) . s) */
/* -------------------------------------- */
/* ctrl: a */
/* envr: e0 */
/* cont: ((eval) (args e0 as ()) (call :nil c) . s) */

/* ctrl: c when (app? c) */
/* envr: e */
/* cont: ((apply e0 :nil) . s) */
/* -------------------------------------- */
/* ctrl: nil */
/* envr: nil */
/* cont: ((call nil c) . s) */
static inline void
wabi_vm_reduce_apply_applicative(const wabi_vm vm,
                                 const wabi_val comb,
                                 const wabi_val env,
                                 const wabi_val args)
{
  wabi_cont cont;
  wabi_val fst, rst;

  if (wabi_is_pair(args)) {
    // arguments to be evaluated
    fst = wabi_car((wabi_pair) args);
    rst = wabi_cdr((wabi_pair) args);

    cont = (wabi_cont) vm->cont;
    cont = wabi_cont_pop(cont);
    cont = wabi_cont_push_call(vm, (wabi_env) env, comb, cont);
    if(vm->ert) return;
    cont = wabi_cont_push_args(vm, (wabi_env) env, rst, vm->emp, cont);
    if(vm->ert) return;
    cont = wabi_cont_push_eval(vm, cont);
    if(vm->ert) return;

    vm->ctrl = (wabi_val) fst;
    vm->env = (wabi_val) env;
    vm->cont = (wabi_val) cont;
  } else {
    // no arguments
    cont = (wabi_cont) vm->cont;
    cont = wabi_cont_pop(cont);
    cont = wabi_cont_push_call(vm, (wabi_env) vm->nil, comb, cont);
    if (vm->ert) return;

    vm->ctrl = (wabi_val) vm->emp;
    vm->env = (wabi_val) vm->nil;
    vm->cont = (wabi_val) cont;
  }
}


static inline void
wabi_vm_reduce_apply(const wabi_vm vm)
{
  wabi_cont_apply cont;
  wabi_val comb, env, args;

  cont = (wabi_cont_apply) vm->cont;
  args = (wabi_val) cont->args;
  env  = (wabi_val) cont->env;
  comb = vm->ctrl;

  #ifdef DEBUG
  if (!wabi_combiner_is_applicative(comb) && !wabi_combiner_is_operative(comb)) {
    vm->ert = wabi_error_other;
    return;
  }
  #endif

  if (wabi_combiner_is_operative(comb)) {
    wabi_vm_reduce_apply_operative(vm, comb, env, args);
    return;
  }

  wabi_vm_reduce_apply_applicative(vm, comb, env, args);
}


/* ctrl: c when (app? c) */
/* envr: e */
/* cont: ((apply e0 (a . as)) . s) */
/* -------------------------------------- */
/* ctrl: a */
/* envr: e0 */
/* cont: ((eval) (args e0 as nil) (call nil c) . s) */
static inline void
wabi_vm_reduce_args_more(const wabi_vm vm)
{
  wabi_cont_args args;
  wabi_cont cont;
  wabi_val fst, rst, cur, done;
  wabi_env envr;
  wabi_pair data;

  args = (wabi_cont_args) vm->cont;
  data = (wabi_pair) args->data;
  envr = (wabi_env) args->env;
  done = (wabi_val) args->done;

  cur = vm->ctrl;
  fst = wabi_car(data);
  rst = wabi_cdr(data);

  done = (wabi_val) wabi_cons(vm, cur, done);
  if(vm->ert) return;

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_args(vm, envr, rst, done, cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->cont = (wabi_val) cont;
  vm->env = (wabi_val) envr;
  vm->ctrl = fst;
}


/* ctrl: x */
/* envr: e */
/* cont: ((args e0 nil xs) . s) */
/* -------------------------------------- */
/* ctrl: (reverse (cons x xs)) */
/* envr: nil */
/* cont: s */
static inline void
wabi_vm_reduce_args_done(const wabi_vm vm)
{
  wabi_cont_args args;
  wabi_cont cont;
  wabi_val vals, done;

  args = (wabi_cont_args) vm->cont;
  done = (wabi_val) args->done;
  vals = (wabi_val) wabi_cons(vm, vm->ctrl, vm->emp);
  if(vm->ert) return;

  vals = wabi_vm_reverse_concat(vm, done, vals);
  if(vm->ert) return;

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  vm->ctrl = (wabi_val) vals;
  vm->cont = (wabi_val) cont;
  // added
  vm->env = vm->nil;
}


static inline void
wabi_vm_reduce_args(const wabi_vm vm)
{
  wabi_cont_args cont;
  wabi_val data;

  cont = (wabi_cont_args) vm->cont;
  data = (wabi_val) cont->data;
  if (wabi_is_pair(data)) {
    wabi_vm_reduce_args_more(vm);
    return;
  }
  wabi_vm_reduce_args_done(vm);
}


/* ctrl: v */
/* envr: e */
/* cont: ((call (cont tag xs)) ... (prompt tag) . ys) */
/* -------------------------------------- */
/* ctrl: v */
/* envr: nil */
/* cont: (xs ... ys) */
static inline void
wabi_vm_reduce_call_continuation(const wabi_vm vm)
{
  wabi_cont_call call;
  wabi_combiner_continuation comb;

  call = (wabi_cont_call) vm->cont;
  comb = (wabi_combiner_continuation) call->combiner;
  wabi_cont_concat_cont(vm, wabi_combiner_continuation_cont(comb));
}


/* ctrl: as */
/* envr: e */
/* cont: ((call e0 #builtin) . s) */
/* -------------------------------------- */
/* (btcall bt as &s) */
static inline void
wabi_vm_reduce_call_builtin(const wabi_vm vm)
{
  wabi_cont_call call;
  wabi_combiner_builtin comb;
  wabi_word func;

  call = (wabi_cont_call) vm->cont;
  comb = (wabi_combiner_builtin) call->combiner;
  func = WABI_WORD_VAL(comb->bid);

  wabi_builtin_call(vm, func);
}

/* ctrl: as */
/* envr: e */
/* cont: ((call e0 (fx e1 ex ps b)) . s) */
/* -------------------------------------- */
/* control b */
/* envr: (bind ex e0 ps as) */
/* cont: ((eval) . s) */
static inline void
wabi_vm_reduce_call_derived(const wabi_vm vm)
{
  wabi_cont_call call;
  wabi_combiner_derived comb;
  wabi_env envr;
  wabi_val body, fst, rst, penv, prms, cenv;
  wabi_cont cont;

  call = (wabi_cont_call) vm->cont;
  comb = (wabi_combiner_derived) call->combiner;
  cenv = (wabi_val) call->env;
  body = (wabi_val) comb->body;
  envr = (wabi_env) comb->static_env;
  penv = (wabi_val) comb->caller_env_name;
  prms = (wabi_val) comb->parameters;

  if (!wabi_is_pair(body)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  fst = wabi_car((wabi_pair) body);
  rst = wabi_cdr((wabi_pair) body);

  envr = wabi_env_extend(vm, WABI_ENV_INITIAL_SIZE, envr);
  if(vm->ert) return;
  wabi_vm_bind(vm, envr, cenv, penv);
  if(vm->ert) return;
  wabi_vm_bind(vm, envr, vm->ctrl, prms);
  if(vm->ert) return;

  cont = (wabi_cont)vm->cont;
  cont = wabi_cont_pop(cont);
  if (wabi_is_pair(rst)) {
    cont = wabi_cont_push_prog(vm, (wabi_env) envr, rst, cont);
    if(vm->ert) return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if (vm->ert) return;

  vm->ctrl = (wabi_val) fst;
  vm->env = (wabi_val) envr;
  vm->cont = (wabi_val) cont;
}

/* ctrl: k */
/* envr: e */
/* cont: ((call e0 {k0 v0 ...}) . s) */
/* -------------------------------------- */
/* control b */
/* envr: (bind ex e0 ps as) */
/* cont: ((eval) . s) */
static inline void
wabi_vm_reduce_call_map(const wabi_vm vm)
{
  wabi_val ctrl, key, val;
  wabi_cont_call call;
  wabi_cont cont;
  wabi_map map;

  call = (wabi_cont_call) vm->cont;
  cont = (wabi_cont) vm->cont;
  map = (wabi_map) call->combiner;
  ctrl = (wabi_val) vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  key = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  val = wabi_map_get(map, key);
  if (!val)  val = vm->nil;

  cont = wabi_cont_pop(cont);
  vm->ctrl = val;
  vm->cont = (wabi_val) cont;
}

/* ctrl: k */
/* envr: e */
/* cont: ((call e0 [x ...]) . s) */
/* -------------------------------------- */
/* control b */
/* envr: (bind ex e0 ps as) */
/* cont: ((eval) . s) */
static inline void
wabi_vm_reduce_call_vector(const wabi_vm vm)
{
  wabi_val ctrl, val;
  wabi_cont_call call;
  wabi_cont cont;
  wabi_vector vec;
  wabi_size s, x;
  wabi_fixnum key;

  call = (wabi_cont_call) vm->cont;
  cont = (wabi_cont) vm->cont;
  vec = (wabi_vector) call->combiner;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  key = (wabi_fixnum) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_atom_is_nil(vm, key)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) wabi_cont_pop(cont);
    return;
  }
  if(!wabi_is_fixnum(key)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  x = WABI_CAST_INT64(key);
  s = wabi_vector_size(vec);

  if(x < 0 || x >= s) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) wabi_cont_pop(cont);
    return;
  }
  val = wabi_vector_ref(vec, x);
  if (!val) {
    vm->ert = wabi_error_other;
    return;
  }
  vm->ctrl = val;
  vm->cont = (wabi_val)wabi_cont_pop(cont);
}

/* ctrl: :nil */
/* envr: e */
/* cont: ((call e0 [x ...]) . s) */
/* -------------------------------------- */
/* ctrl :nil */
/* envr: e */
/* cont: s */
static inline void
wabi_vm_reduce_call_nil(const wabi_vm vm)
{
  wabi_cont cont;;

  cont = (wabi_cont) vm->cont;
  vm->ctrl = vm->nil;
  vm->cont = (wabi_val)wabi_cont_pop(cont);
}

static inline void
wabi_vm_reduce_call(const wabi_vm vm)
{
  wabi_cont_call call;
  wabi_val comb;

  call = (wabi_cont_call) vm->cont;
  comb = (wabi_val) call->combiner;
  switch (WABI_TAG(comb)) {

  case wabi_tag_ct:
    wabi_vm_reduce_call_continuation(vm);
    break;

  case wabi_tag_bt_oper:
  case wabi_tag_bt_app:
    wabi_vm_reduce_call_builtin(vm);
    break;

  case wabi_tag_oper:
  case wabi_tag_app:
    wabi_vm_reduce_call_derived(vm);
    break;

  case wabi_tag_map_hash:
  case wabi_tag_map_array:
  case wabi_tag_map_entry:
    wabi_vm_reduce_call_map(vm);
    break;

  case wabi_tag_vector_deep:
  case wabi_tag_vector_digit:
    wabi_vm_reduce_call_vector(vm);
    break;
  default:
    if (comb == vm->nil) {
      wabi_vm_reduce_call_nil(vm);
      break;
    }
    // todo: vectors and maps managed as applicatives
    vm->ert = wabi_error_type_mismatch;
  }
}


/* ctrl: false | nil */
/* envr: e */
/* cont: ((sel e0 l r) . s) */
/* -------------------------------------- */
/* ctrl: r */
/* envr: e0 */
/* cont: ((eval) . s) */

/* ctrl: _ */
/* envr: e */
/* cont: ((sel e0 l r) . s) */
/* -------------------------------------- */
/* ctrl: l */
/* envr: e0 */
/* cont: ((eval) . s) */
static inline void
wabi_vm_reduce_sel(const wabi_vm vm)
{
  wabi_cont_sel sel;
  int falsey;
  wabi_val lft, rgt, env;
  wabi_cont cont;

  sel = (wabi_cont_sel) vm->cont;
  lft = (wabi_val) sel->left;
  rgt = (wabi_val) sel->right;
  env = (wabi_val) sel->env;

  falsey = wabi_atom_is_false(vm, vm->ctrl) || wabi_atom_is_nil(vm, vm->ctrl);
  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) (falsey ? rgt : lft);
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}


static inline void
wabi_vm_reduce_prog(const wabi_vm vm)
{
  wabi_cont_prog prog;
  wabi_env env;
  wabi_cont cont;
  wabi_val exps, fst, rst;

  prog = (wabi_cont_prog) vm->cont;
  env = (wabi_env) prog->env;
  exps = (wabi_val) prog->expressions;

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  if(!wabi_is_pair(exps)) {
    vm->cont = (wabi_val) cont;
    vm->env = (wabi_val) vm->nil;
    return;
  }
  fst = wabi_car((wabi_pair) exps);
  rst = wabi_cdr((wabi_pair) exps);

  if(wabi_is_pair(rst)) {
    cont = wabi_cont_push_prog(vm, env, rst, cont);
    if(vm->ert) return;
  }

  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) fst;
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}


static inline void
wabi_vm_reduce_def(const wabi_vm vm)
{
  wabi_cont_def def;
  wabi_env env;
  wabi_val patt;
  wabi_cont cont;

  def = (wabi_cont_def) vm->cont;
  env = (wabi_env) def->env;
  patt = (wabi_val) def->pattern;

  wabi_vm_bind(vm, env, vm->ctrl, patt);
  if(vm->ert) return;

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  vm->cont = (wabi_val) cont;
}


void
wabi_vm_run(const wabi_vm vm,
            const wabi_size fuel)
{
  static void *cont_ptrs[] = { &&eval, &&prompt, &&apply, &&call, &&sel, &&args, &&def, &&prog };
  static void *err_ptrs[] = { &&none, &&nomem, &&err, &&err, &&err, &&err, &&err, &&err, &&err, &&err, &&err, &&err, &&err  };

  vm->fuel = fuel;
  vm->ert = wabi_error_none;

 next:
  /* printf("ctrl: "); wabi_prn(vm, vm->ctrl); */
  /* printf("cont: "); wabi_prn(vm, vm->cont); */
  goto *err_ptrs[vm->ert];

 nomem:
  wabi_vm_collect(vm);
  if (vm->ert) return;
  goto none;

 err:
  return;

 none:
  if (vm->fuel <= 0) {
    vm->ert = wabi_error_timeout;
    return;
  }
  if (vm->cont) {
    goto *cont_ptrs[(*(vm->cont) >> 56) - 0x13];
  }
  return;

 eval:
  wabi_vm_reduce_eval(vm);
  goto next;

 apply:
  wabi_vm_reduce_apply(vm);
  goto next;

 prompt:
  wabi_vm_reduce_prompt(vm);
  goto next;

 args:
  wabi_vm_reduce_args(vm);
  goto next;

 call:
  wabi_vm_reduce_call(vm);
  goto next;

 sel:
  wabi_vm_reduce_sel(vm);
  goto next;

 prog:
  wabi_vm_reduce_prog(vm);
  goto next;

 def:
  wabi_vm_reduce_def(vm);
  goto next;
}
