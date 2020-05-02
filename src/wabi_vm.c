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

#include <stdint.h>
#include <stddef.h>
#include "wabi_vm.h"
#include "wabi_store.h"
#include "wabi_system.h"
#include "wabi_collect.h"
#include "wabi_cont.h"
#include "wabi_pair.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_binary.h"
#include "wabi_symbol.h"
#include "wabi_constant.h"


static inline wabi_val
wabi_vm_nil(wabi_vm vm)
{
  wabi_val v = wabi_vm_alloc(vm, WABI_CONSTANT_SIZE);
  if(v) {
    *v = wabi_val_nil;
  }
  return v;
}


static inline wabi_val
wabi_vm_declare_other_sym(wabi_vm vm, char* cstr, wabi_val res)
{
  wabi_binary_leaf bin;
  wabi_val sym;
  int j = 0;
  bin = wabi_binary_leaf_new_from_cstring(vm, cstr);
  if(! bin) return NULL;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(! sym) return NULL;
  res = (wabi_val) wabi_cons(vm, sym, res);
  if(! res) return NULL;
  return res;
}

static inline wabi_val
wabi_vm_others(wabi_vm vm, wabi_val res)
{
  res = wabi_vm_declare_other_sym(vm, "q", res);
  if(! res) return NULL;
  res = wabi_vm_declare_other_sym(vm, "hmap", res);
  return res;
}


int
wabi_vm_init(wabi_vm vm, wabi_size size)
{
  wabi_val stbl, nil, oth;
  if(wabi_store_init(&(vm->stor), size))
    return 1;

  stbl = (wabi_val) wabi_map_empty(vm);
  if(! stbl) return 2;

  nil = wabi_vm_nil(vm);
  if(!nil) return 3;

  vm->stbl  = stbl;

  oth = wabi_vm_others(vm, nil);
  if(!oth) return 4;


  vm->ctrl = NULL;
  vm->env = NULL;
  vm->cont = (wabi_val) wabi_cont_done;
  vm->prmt = (wabi_val) wabi_cont_done;
  vm->nil = nil;
  vm->oth = oth;
  vm->fuel = 0;
  vm->erv = nil;
  vm->ert = wabi_error_none;
  return 0;
}


int
wabi_vm_collect(wabi_vm vm)
{
  int res;

  wabi_store_prepare(&vm->stor);

  if(vm->ctrl) vm->ctrl = wabi_copy_val(vm, vm->ctrl);
  if(vm->env) vm->env = wabi_copy_val(vm, vm->env);
  if(vm->cont) vm->cont = wabi_copy_val(vm, vm->cont);
  if(vm->prmt) vm->prmt = wabi_copy_val(vm, vm->prmt);
  /* vm->stbl = (wabi_val) wabi_map_empty(vm); */
  /* if(!vm->stbl) return 1; */
  if(vm->stbl) vm->stbl = wabi_copy_val(vm, vm->stbl);
  if(vm->nil) vm->nil = wabi_copy_val(vm, vm->nil);
  if(vm->oth) vm->oth = wabi_copy_val(vm, vm->oth);

  wabi_collect(vm);
  vm->ert = wabi_error_none;
  return 0;
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
wabi_vm_reverse(wabi_vm vm,
                wabi_val done,
                wabi_val res)
{
  while(WABI_IS(wabi_tag_pair, done)) {
    res = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) done), res);
    if(res) {
      done = wabi_cdr((wabi_pair) done);
      continue;
    }
    return NULL;
  }
  return res;
}


inline static wabi_error_type
wabi_vm_bind(wabi_vm vm,
             wabi_env env,
             wabi_val args,
             wabi_val params)
{
  int partial;
  if(WABI_IS(wabi_tag_symbol, params)) {
    return wabi_env_set(vm, env, (wabi_symbol) params, args);
  }
  if(*params == wabi_val_ignore) {
    return wabi_error_none;
  }
  if(WABI_IS(wabi_tag_pair, params)) {
    if(WABI_IS(wabi_tag_pair, args)) {
      partial = wabi_vm_bind(vm, env, wabi_car((wabi_pair) args), wabi_car((wabi_pair) params));
      if(partial != wabi_error_none) return partial;
      return wabi_vm_bind(vm, env, wabi_cdr((wabi_pair) args), wabi_cdr((wabi_pair) params));
    }
    if(*args == wabi_val_nil) {
      partial = wabi_vm_bind(vm, env, vm->nil, wabi_car((wabi_pair) params));
      if(partial != wabi_error_none) return partial;
      return wabi_vm_bind(vm, env, vm->nil, wabi_cdr((wabi_pair) params));
    }
  }
  if(wabi_cmp(params, args) == 0) {
    return wabi_error_none;
  }
  return wabi_error_bindings;
}

static inline void
wabi_vm_reduce(wabi_vm vm)
{
  wabi_val ctrl, ctrl0;
  wabi_cont cont, cont0;
  wabi_env env;
  wabi_pair pair;

  cont = (wabi_cont) vm->cont;
  ctrl = (wabi_val) vm->ctrl;
  env  = (wabi_env) vm->env;

  if(!cont) {
    /* control x0 */
    /* envr: e */
    /* cont: nil */
    /* -------------------------------------- */
    /* WIN! */
    return;
  }
  switch(WABI_TAG(cont)) {
  case wabi_tag_cont_prompt:
    vm->cont = (wabi_val) wabi_cont_next(cont);
    vm->prmt = (wabi_val) wabi_cont_prompt_next_prompt((wabi_cont_prompt) cont);
    return;
  case wabi_tag_cont_eval:
    /* ctrl: (f . as) */
    /* envr: e */
    /* cont: ((eval) . s) */
    /*       -------------------------------------- */
    /* ctrl: f */
    /* envr: e */
    /* cont: ((eval) (apply e0 as) . s) */
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      pair = (wabi_pair) ctrl;
      ctrl0 = wabi_cdr(pair);
      cont0 = wabi_cont_next(cont);
      cont0 = wabi_cont_push_apply(vm, env, ctrl0, cont0);

      if(cont0) {
        cont0 = wabi_cont_push_eval(vm, cont0);
        if(cont0) {
          vm->ctrl = wabi_car(pair);
          vm->cont = (wabi_val) cont0;
          return;
        }
      }
      vm->ert = wabi_error_nomem;
      return;
    }
    /* ctrl: c when (sym? c) */
    /* envr: e */
    /* cont: ((eval) . s) */
    /* -------------------------------------- */
    /* ctrl: (lookup c e) */
    /* envr: e */
    /* cont: s */
    if(WABI_IS(wabi_tag_symbol, ctrl)) {
      ctrl0 = wabi_env_lookup(env, (wabi_symbol) ctrl);
      if(ctrl0) {
        vm->ctrl = ctrl0;
        vm->cont = (wabi_val) wabi_cont_next(cont);
        return;
      }
      vm->ert = wabi_error_unbound_name;
      return;
    }
    /* ctrl: c */
    /* envr: e */
    /* cont: ((eval) . s) */
    /* -------------------------------------- */
    /* ctrl: c */
    /* envr: e */
    /* cont: s */
    vm->cont = (wabi_val) wabi_cont_next(cont);
    return;

  case wabi_tag_cont_apply:
    /* ctrl: c when (oper? c) */
    /* envr: e */
    /* cont: ((apply e0 as) . s) */
    /* -------------------------------------- */
    /* ctrl: as */
    /* envr: e0 */
    /* cont: ((call e0 c) . s) */
    if(wabi_combiner_is_operative(ctrl)) {
      cont0 = wabi_cont_next(cont);
      cont0 = wabi_cont_push_call(vm, (wabi_env) ((wabi_cont_apply) cont)->env, ctrl, cont0);
      if(cont0) {
        ctrl0 = (wabi_val) ((wabi_cont_apply) cont)->args;
        vm->ctrl = ctrl0;
        vm->env = (wabi_val) ((wabi_cont_apply) cont)->env;
        vm->cont = (wabi_val) cont0;
        return;
      }
      vm->ert = wabi_error_nomem;
      return;
    }
    if(wabi_combiner_is_applicative(ctrl)) {

      /* ctrl: c when (app? c) */
      /* envr: e */
      /* cont: ((apply e0 nil) . s) */
      /* -------------------------------------- */
      /* ctrl: nil */
      /* envr: nil */
      /* cont: ((call nil c) . s) */
      if(*((wabi_val) (((wabi_cont_apply) cont)->args)) == wabi_val_nil) {
        cont0 = wabi_cont_next(cont);
        cont0 = wabi_cont_push_call(vm, (wabi_env) vm->nil, ctrl, cont0);
        if(cont0) {
          vm->ctrl = vm->nil;
          vm->env = vm->nil;
          vm->cont = (wabi_val) cont0;
          return;
        }
        vm->ert = wabi_error_nomem;
        return;
      }
      /* ctrl: c when (app? c) */
      /* envr: e */
      /* cont: ((apply e0 (a . as)) . s) */
      /* -------------------------------------- */
      /* ctrl: a */
      /* envr: e0 */
      /* cont: ((eval) (args e0 as nil) (call nil c) . s) */
      pair = (wabi_pair) ((wabi_cont_apply) cont)->args;
      cont0 = wabi_cont_next(cont);
      cont0 = wabi_cont_push_call(vm, (wabi_env) vm->nil, ctrl, cont0);
      if(cont0) {
        cont0 = wabi_cont_push_args(vm, (wabi_env) ((wabi_cont_apply) cont)->env, wabi_cdr(pair), vm->nil, cont0);
        if(cont0) {
          cont0 = wabi_cont_push_eval(vm, cont0);
          if(cont0) {
            vm->ctrl = wabi_car(pair);
            vm->env = (wabi_val) ((wabi_cont_apply) cont)->env;
            vm->cont = (wabi_val) cont0;
            return;
          }
        }
      }
      vm->ert = wabi_error_nomem;
      return;
    }
    vm->ert = wabi_error_other;
    return;

  case wabi_tag_cont_args:
    /* ctrl: c when (app? c) */
    /* envr: e */
    /* cont: ((apply e0 (a . as)) . s) */
    /* -------------------------------------- */
    /* ctrl: a */
    /* envr: e0 */
    /* cont: ((eval) (args e0 as nil) (call nil c) . s) */
    if(WABI_IS(wabi_tag_pair, ((wabi_cont_args) cont)->data)) {
      pair = (wabi_pair) ((wabi_cont_args) cont)->data;
      cont0 = wabi_cont_next(cont);
      ctrl0 = (wabi_val) wabi_cons(vm, ctrl, (wabi_val) ((wabi_cont_args) cont)->done);
      if(ctrl0) {
        cont0 = wabi_cont_push_args(vm,
                                    (wabi_env) ((wabi_cont_args) cont)->env,
                                    wabi_cdr(pair),
                                    ctrl0,
                                    cont0);
        if(cont0) {
          cont0 = wabi_cont_push_eval(vm, cont0);
          if(cont0) {
            vm->cont = (wabi_val) cont0;
            vm->env = (wabi_val) ((wabi_cont_args) cont)->env;
            vm->ctrl = wabi_car(pair);
            return;
          }
        }
      }
      vm->ert = wabi_error_nomem;
      return;
    }

    /* ctrl: x */
    /* envr: e */
    /* cont: ((args e0 nil xs) . s) */
    /* -------------------------------------- */
    /* ctrl: (reverse (cons x xs)) */
    /* envr: nil */
    /* cont: s */
    ctrl0 = (wabi_val) wabi_cons(vm, ctrl, vm->nil);
    if(ctrl0) {
      ctrl0 = wabi_vm_reverse(vm,(wabi_val) ((wabi_cont_args) cont)->done, ctrl0);
      if(ctrl0) {
        vm->ctrl = ctrl0;
        vm->env = vm->nil; // (wabi_val) ((wabi_cont_args) cont)->env;
        vm->cont = (wabi_val) wabi_cont_next(cont);
        return;
      }
    }
    vm->ert = wabi_error_nomem;
    return;

  case wabi_tag_cont_call:
    /***** CONT *****/
    if(wabi_combiner_is_continuation((wabi_val) ((wabi_cont_call) cont)->combiner)) {
      wabi_cont_concat_cont(vm, wabi_combiner_continuation_cont((wabi_combiner_continuation) ((wabi_cont_call) cont)->combiner));
      return;
    }

    /* ctrl: as */
    /* envr: e */
    /* cont: ((call e0 (fx e1 ex ps b)) . s) */
    /* -------------------------------------- */
    /* control b */
    /* envr: (bind ex e0 ps as) */
    /* cont: ((eval) . s) */
    if(wabi_combiner_is_derived((wabi_val) ((wabi_cont_call) cont)->combiner)) {
      ctrl0 = (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->compiled_body;
      if(*ctrl0 == wabi_val_nil) {
        ctrl0 = (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->body;
      }
      if(WABI_IS(wabi_tag_pair, ctrl0)) {
        env = (wabi_env) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->static_env;
        env = wabi_env_extend(vm, env);
        if(env) {
          vm->ert = wabi_vm_bind(vm, env, (wabi_val) ((wabi_cont_call) cont)->env, (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->caller_env_name);
          if(vm->ert) {
            return;
          }
          vm->ert = wabi_vm_bind(vm, env, ctrl, (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->parameters);
          if(vm->ert) {
            return;
          }
          cont0 = wabi_cont_next(cont);
          if(WABI_IS(wabi_tag_pair, wabi_cdr((wabi_pair) ctrl0))) {
            cont0 = wabi_cont_push_prog(vm, env, wabi_cdr((wabi_pair) ctrl0), cont0);
            if(cont0) {
              cont0 = wabi_cont_push_eval(vm, cont0);
              if(cont0) {
                vm->ctrl = wabi_car((wabi_pair) ctrl0);
                vm->env = (wabi_val) env;
                vm->cont = (wabi_val) cont0;
                return;
              }
            }
            vm->ert = wabi_error_nomem;
            return;
          } else {
            cont0 = wabi_cont_push_eval(vm, cont0);
            if(cont0) {
              vm->ctrl = wabi_car((wabi_pair) ctrl0);
              vm->env = (wabi_val) env;
              vm->cont = (wabi_val) cont0;
              return;
            }
            vm->ert = wabi_error_nomem;
            return;
          }
        }
        vm->ert = wabi_error_nomem;
        return;
      }
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    /* ctrl: as */
    /* envr: e */
    /* cont: ((call e0 #builtin) . s) */
    /* -------------------------------------- */
    /* (btcall bt as &s) */
    if(wabi_combiner_is_builtin((wabi_val) ((wabi_cont_call) cont)->combiner)) {
      // todo: do not return error state
      vm->ert = ((wabi_builtin_fun) (WABI_WORD_VAL(((wabi_combiner_builtin) ((wabi_cont_call) cont)->combiner)->c_ptr)))(vm);
      return;
    }
    // things that implements invoke

    vm->ert = wabi_error_type_mismatch;
    return;

    case wabi_tag_cont_sel:
    if(*(ctrl) == wabi_val_false || *(ctrl) == wabi_val_nil) {
      /* ctrl: false | nil */
      /* envr: e */
      /* cont: ((sel e0 l r) . s) */
      /* -------------------------------------- */
      /* ctrl: r */
      /* envr: e0 */
      /* cont: ((eval) . s) */
      cont0 = wabi_cont_next(cont);
      cont0 = wabi_cont_push_eval(vm, cont0);
      if(cont0) {
        vm->ctrl = (wabi_val) ((wabi_cont_sel) cont)->right;
        vm->env = (wabi_val) ((wabi_cont_sel) cont)->env;
        vm->cont = (wabi_val) cont0;
        return;
      }
      vm->ert =  wabi_error_nomem;
      return;
    }
    /* ctrl: _ */
    /* envr: e */
    /* cont: ((sel e0 l r) . s) */
    /* -------------------------------------- */
    /* ctrl: l */
    /* envr: e0 */
    /* cont: ((eval) . s) */
    cont0 = wabi_cont_next(cont);
    cont0 = wabi_cont_push_eval(vm, cont0);
    if(cont0) {
      vm->ctrl = (wabi_val) ((wabi_cont_sel) cont)->left;
      vm->env = (wabi_val) ((wabi_cont_sel) cont)->env;
      vm->cont = (wabi_val) cont0;
      return;
    }
    vm->ert = wabi_error_nomem;
    return;
  case wabi_tag_cont_prog:
    /* ctrl: x0 */
    /* envr: e */
    /* cont: ((prog e0 nil) . s) */
    /* -------------------------------------- */
    /* ctrl: x0 */
    /* envr: nil */
    /* cont: s */
    if(*((wabi_val) ((wabi_cont_prog) cont)->expressions) == wabi_val_nil) {
      vm->cont = (wabi_val) wabi_cont_next(cont);
      vm->env = vm->nil;
      return;
    }
    /* ctrl: x0 */
    /* envr: e */
    /* cont: ((prog e0 (x . xs)) . s) */
    /* -------------------------------------- */
    /* ctrl: x */
    /* envr: e0 */
    /* cont: ((prog e0 xs) . s) */
    cont0 = wabi_cont_next(cont);
    pair = (wabi_pair) ((wabi_cont_prog) cont)->expressions;
    env = (wabi_env) ((wabi_cont_prog) cont)->env;
    cont0 = wabi_cont_push_prog(vm, env, wabi_cdr(pair), cont0);
    if(cont0) {
      cont0 = wabi_cont_push_eval(vm, cont0);
      if(cont0) {
        vm->ctrl = (wabi_val) wabi_car(pair);
        vm->env = (wabi_val) env;
        vm->cont = (wabi_val) cont0;
        return;
      }
    }
    return;

  case wabi_tag_cont_def:
    /* ctrl: as */
    /* envr: e */
    /* cont: ((def e0 ps) . s) */
    /* -------------------------------------- */
    /* ctrl: (last vs) */
    /* envr: e0 */
    /* cont: s // (bind e0 ps as) */
    vm->ert = wabi_vm_bind(vm, (wabi_env) ((wabi_cont_def) cont)->env, ctrl, (wabi_val) ((wabi_cont_def) cont)->pattern);
    if(vm->ert) {
      return;
    }
    vm->cont = (wabi_val) wabi_cont_next(cont);
    return;
  }
  vm->ert = wabi_error_other;
  return;
}


#define WABI_REDUCTIONS_LIMIT 1000000

void
wabi_vm_run(wabi_vm vm, wabi_size fuel) {
  vm->fuel = fuel;
  vm->ert = wabi_error_none;

  for(;;) {
    wabi_vm_reduce(vm);

    if((wabi_cont) vm->cont == wabi_cont_done) {
      return;
    }
    if(vm->fuel < 0) {
      vm->ert = wabi_error_timeout;
      return;
    }
    if(! vm->ert)
      continue;
    if(vm->ert == wabi_error_nomem && wabi_vm_collect(vm)) {
      vm->ert = wabi_error_none;
      continue;
    }
  }
}
