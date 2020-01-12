#define wabi_vm_c

#include <stdint.h>
#include <stddef.h>
#include "wabi_vm.h"
#include "wabi_store.h"
#include "wabi_cont.h"
#include "wabi_combiner.h"
#include "wabi_system.h"
#include "wabi_value.h"
#include "wabi_pair.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_cmp.h"
#include "wabi_number.h"
#include "wabi_combiner.h"
#include "wabi_store.h"
#include "wabi_error.h"

/*** TMP ***/
#include <stdio.h>
#include "wabi_pr.h"


#define WABI_VM_DEBUG 1


static inline wabi_val
wabi_vm_nil(wabi_vm vm)
{
  wabi_val v = wabi_vm_alloc(vm, 1);
  if(v) {
    *v = wabi_val_nil;
  }
  return v;
}


int
wabi_vm_collect(wabi_vm vm)
{
  wabi_store store;
  int res;

  store = &(vm->store);
  if(wabi_store_collect_prepare(store)) {
    if(vm->control) vm->control = wabi_store_copy_val(store, vm->control);
    if(vm->continuation) vm->continuation = wabi_store_copy_val(store, vm->continuation);
    // todo: do not copy the symbol table (it. should be fed during gc);
    // vm->symbol_table = (wabi_word) wabi_map_empty();
    if(vm->symbol_table) vm->symbol_table = wabi_store_copy_val(store, vm->symbol_table);
    res = wabi_store_collect(store);
    if(res) {
      return res;
    }
  }
  return 0;
}

static inline int
wabi_vm_free_words(wabi_vm vm)
{
  wabi_store store;
  store = &(vm->store);
  return wabi_store_free_words(store);
}

int
wabi_vm_init(wabi_vm vm, wabi_size store_size)
{
  int store_init;
  wabi_val symbol_table;

  vm->fuel = 100000;
  vm->continuation = NULL;
  vm->symbol_table = NULL;
  store_init = wabi_store_init(&(vm->store), store_size);
  if(store_init) {
    symbol_table = (wabi_val) wabi_map_empty(vm);
    if(symbol_table) {
      vm->symbol_table = symbol_table;
      return store_init;
    }
    return wabi_error_nomem;
  }
  return store_init;
}


void
wabi_vm_destroy(wabi_vm vm)
{
  wabi_store_destroy(&(vm->store));
}


wabi_val
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
  if(WABI_IS(wabi_tag_pair, params) && WABI_IS(wabi_tag_pair, args)) {
    partial = wabi_vm_bind(vm, env, wabi_car((wabi_pair) args), wabi_car((wabi_pair) params));
    if(partial != wabi_error_none) return partial;
    return wabi_vm_bind(vm, env, wabi_cdr((wabi_pair) args), wabi_cdr((wabi_pair) params));
  }
  if(wabi_cmp(params, args) == 0) {
    return wabi_error_none;
  }
  return wabi_error_bindings;
}

// static inline wabi_error_type
wabi_error_type
wabi_vm_reduce(wabi_vm vm)
{
  wabi_val ctrl, ctrl0;
  wabi_cont cont, cont0;
  wabi_env env;
  wabi_error_type err;

  cont = (wabi_cont) vm->continuation;
  ctrl = (wabi_val) vm->control;
  if(!cont) {
    /* control x0 */
    /* stack: nil */
    /* -------------------------------------- */
    /* WIN! */
    return wabi_error_done;
  }

  switch(WABI_TAG(cont)) {
  case wabi_tag_cont_prompt:
  case wabi_tag_cont_eval:

    /* control: (f . as) */
    /* stack: ((eval e0) . s) */
    /* -------------------------------------- */
    /* control: f */
    /* stack: ((eval e0) (apply e0 as) . s) */
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_apply(vm, (wabi_env) ((wabi_cont_eval) cont)->env, wabi_cdr((wabi_pair) ctrl), cont0);
      if(cont0) {
        cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_eval) cont)->env, cont0);
        if(cont0) {
          vm->control = wabi_car((wabi_pair) ctrl);
          vm->continuation = (wabi_val) cont0;
          return wabi_error_none;
        }
      }
      return wabi_error_nomem;
    }

    /* control: c when (sym? c) */
    /* stack: ((eval e0) . s) */
    /* -------------------------------------- */
    /* control: (lookup c e0) */
    /* stack s */
    if(WABI_IS(wabi_tag_symbol, ctrl)) {
      ctrl0 = wabi_env_lookup((wabi_env) ((wabi_cont_eval) cont)->env, (wabi_symbol) ctrl);
      if(ctrl0) {
        cont0 = wabi_cont_prev(cont);

        vm->control = ctrl0;
        vm->continuation = (wabi_val) cont0;
        return wabi_error_none;
      }
      return wabi_error_unbound_name;
    }

    /* control: c */
    /* stack: ((eval e0) . s) */
    /* -------------------------------------- */
    /* control: c */
    /* stack s */
    cont0 = wabi_cont_prev(cont);
    vm->continuation = (wabi_val)cont0;
    return wabi_error_none;

  case wabi_tag_cont_apply:

    /* control: c */
    /* stack: ((apply e0 nil) . s) */
    /* -------------------------------------- */
    /* control: nil */
    /* stack: ((call e0 c) . s) */
    if(*((wabi_val) (((wabi_cont_apply) cont)->args)) == wabi_val_nil) {
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_call(vm, (wabi_env) ((wabi_cont_apply) cont)->env, ctrl, cont0);
      if(cont0) {
        ctrl0 = (wabi_val) ((wabi_cont_apply) cont)->args;
        vm->control = ctrl0;
        vm->continuation = (wabi_val) cont0;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
    /* control: c when (oper? c) */
    /* stack: ((apply e0 as) . s) */
    /* -------------------------------------- */
    /* control: as */
    /* stack: ((call e0 c) . s) */
    if(wabi_combiner_is_operative(ctrl)) {
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_call(vm, (wabi_env) ((wabi_cont_apply) cont)->env, ctrl, cont0);
      if(cont0) {
        ctrl0 = (wabi_val) ((wabi_cont_apply) cont)->args;
        vm->control = ctrl0;
        vm->continuation = (wabi_val) cont0;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }

    /* control: c when (app? c) */
    /* stack: ((apply e0 (a . as)) . s) */
    /* -------------------------------------- */
    /* control: a */
    /* stack: ((eval e0) (eval-more e0 as nil) (call e0 c) . s) */
    if(wabi_combiner_is_applicative(ctrl) && WABI_IS(wabi_tag_pair, ((wabi_cont_apply) cont)->args)) {
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_call(vm, (wabi_env) ((wabi_cont_apply) cont)->env, ctrl, cont0);
      if(cont0) {
        cont0 = wabi_cont_push_eval_more(vm, (wabi_env) ((wabi_cont_apply) cont)->env, wabi_cdr((wabi_pair) ((wabi_cont_apply) cont)->args), wabi_vm_nil(vm), cont0);
        if(cont0) {
          cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_apply) cont)->env, cont0);
          if(cont0) {
            ctrl0 = wabi_car((wabi_pair) ((wabi_cont_apply) cont)->args);
            vm->control = ctrl0;
            vm->continuation = (wabi_val) cont0;
            return wabi_error_none;
          }
        }
      }
      return wabi_error_nomem;
    }
    return wabi_error_other;

  case wabi_tag_cont_eval_more:
    /* control: x */
    /* stack: ((eval-more e0 (a . as) xs) . s) */
    /* -------------------------------------- */
    /* control: a */
    /* stack: ((eval e0) (eval-more e0 as (cons x xs)) . s) */
    if(WABI_IS(wabi_tag_pair, ((wabi_cont_eval_more) cont)->data)) {
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_eval_more(vm,
                                       (wabi_env) ((wabi_cont_eval_more) cont)->env,
                                       wabi_cdr((wabi_pair) ((wabi_cont_eval_more) cont)->data),
                                       (wabi_val) wabi_cons(vm, ctrl, (wabi_val) ((wabi_cont_eval_more) cont)->done),
                                       cont0);
      if(cont0) {
        cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_eval_more) cont)->env, cont0);
        if(cont0) {
          ctrl0 = wabi_car((wabi_pair) ((wabi_cont_eval_more) cont)->data);
          vm->continuation = (wabi_val) cont0;
          vm->control = ctrl0;
          return wabi_error_none;
        }
      }
      return wabi_error_nomem;
    }
    /* control: x */
    /* stack: ((eval-more e0 nil xs) . s) */
    /* -------------------------------------- */
    /* control: (reverse (cons x xs)) */
    /* stack: s */
    ctrl0 = wabi_vm_nil(vm);
    if(ctrl0) {
      ctrl0 = (wabi_val) wabi_cons(vm, ctrl, ctrl0);
      if(ctrl0) {
        ctrl0 = wabi_vm_reverse(vm,(wabi_val) ((wabi_cont_eval_more) cont)->done, ctrl0);
        if(ctrl0) {
          cont0 = wabi_cont_prev(cont);
          vm->control = ctrl0;
          vm->continuation = (wabi_val) cont0;
          return wabi_error_none;
        }
      }
    }
    return wabi_error_nomem;

  case wabi_tag_cont_call:
    /* control: as */
    /* stack: ((call e0 (fx e1 ex ps b)) . s) */
    /* -------------------------------------- */
    /* control b */
    /* stack: ((eval (bind ex e0 ps as)) . s) */
    if(wabi_combiner_is_derived((wabi_val) ((wabi_cont_call) cont)->combiner)) {
      env = wabi_env_extend(vm, (wabi_env) ((wabi_cont_call) cont)->env);
      if(env) {
        err = wabi_vm_bind(vm, env, (wabi_val) ((wabi_cont_call) cont)->env, (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->caller_env_name);
        if(err) return err;
        err = wabi_vm_bind(vm, env, ctrl, (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->parameters);
        if(err) return err;
        cont0 = wabi_cont_prev(cont);
        cont0 = wabi_cont_push_eval(vm, env, cont0);
        if(cont0) {
          ctrl0 = (wabi_val) ((wabi_combiner_derived) ((wabi_cont_call) cont)->combiner)->body;
          vm->control = ctrl0;
          vm->continuation = (wabi_val) cont0;
          return wabi_error_none;
        }
      }
      return wabi_error_nomem;
    }
    /* control: as */
    /* stack: ((call e0 #builtin) . s) */
    /* -------------------------------------- */
    /* control: (btcall bt as &s) */
    /* stack: s */
    if(wabi_combiner_is_builtin((wabi_val) ((wabi_cont_call) cont)->combiner)) {
      // todo do not call with env?
      return ((wabi_builtin_fun) (WABI_WORD_VAL(((wabi_combiner_builtin) ((wabi_cont_call) cont)->combiner)->c_ptr)))(vm);
    }
    // things that implements invoke
    return wabi_error_type_mismatch;

  case wabi_tag_cont_sel:
    if(*(ctrl) == wabi_val_false || *(ctrl) == wabi_val_nil) {
      /* control: false | nil */
      /* stack: ((sel e0 l r) . s) */
      /* -------------------------------------- */
      /* control: r */
      /* stack: ((eval e0) . s) */
      cont0 = wabi_cont_prev(cont);
      cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_sel) cont)->env, cont0);
      if(cont0) {
        ctrl0 = (wabi_val) ((wabi_cont_sel) cont)->right;
        vm->control = ctrl0;
        vm->continuation = (wabi_val) cont0;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
    /* control: _ */
    /* stack: ((sel e0 l r) . s) */
    /* -------------------------------------- */
    /* control: l */
    /* stack: ((eval e0) . s) */
    cont0 = wabi_cont_prev(cont);
    cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_sel) cont)->env, cont0);
    if(cont0) {
      ctrl0 = (wabi_val) ((wabi_cont_sel) cont)->left;
      vm->control = ctrl0;
      vm->continuation = (wabi_val) cont0;
      return wabi_error_none;
    }
    return wabi_error_nomem;

  case wabi_tag_cont_prog:
    /* control: x0 */
    /* stack: ((prog e0 nil) . s) */
    /* -------------------------------------- */
    /* control: x0 */
    /* stack: s */
    if(  *((wabi_val) ((wabi_cont_prog) cont)->expressions) == wabi_val_nil) {
      cont0 = wabi_cont_prev(cont);
      vm->continuation = (wabi_val) cont0;
      return wabi_error_none;
    }
    /* control: x0 */
    /* stack: ((prog e0 (x . xs)) . s) */
    /* -------------------------------------- */
    /* control: x */
    /* stack: ((eval e0) (prog e0 xs) . s) */
    cont0 = wabi_cont_prev(cont);
    cont0 = wabi_cont_push_prog(vm, (wabi_env) ((wabi_cont_prog) cont)->env, wabi_cdr((wabi_pair) ((wabi_cont_prog) cont)->expressions), cont0);
    if(cont0) {
      cont0 = wabi_cont_push_eval(vm, (wabi_env) ((wabi_cont_prog) cont)->env, cont0);
      if(cont0) {
        ctrl0 = wabi_car((wabi_pair) (((wabi_cont_prog) cont)->expressions));
        vm->control = ctrl0;
        vm->continuation = (wabi_val) cont0;
        return wabi_error_none;
      }
    }
    return wabi_error_nomem;

  case wabi_tag_cont_def:

    /* control: as */
    /* stack: ((def e0 ps) . s) */
    /* -------------------------------------- */
    /* control: as */
    /* stack: s // (bind e0 ps as) */
    err = wabi_vm_bind(vm, (wabi_env) ((wabi_cont_def) cont)->env, ctrl, (wabi_val) ((wabi_cont_def) cont)->pattern);
    if(err) return err;
    cont0 = wabi_cont_prev(cont);
    vm->continuation = (wabi_val) cont0;
    return wabi_error_none;
  }
  return wabi_error_other;
}


#define WABI_REDUCTIONS_LIMIT 1000000

wabi_error_type
wabi_vm_run(wabi_vm vm) {
  wabi_error_type err;
  wabi_size reductions;
  reductions = WABI_REDUCTIONS_LIMIT;
  do {
    reductions--;

    /* printf("mem: %lu/%lu\n", (vm->store.heap - vm->store.space), vm->store.size); */
    /* printf("c: "); */
    /* wabi_pr(vm->control); */
    /* printf("\nk: "); */
    /* if(vm->continuation) wabi_pr(vm->continuation); */
    /* printf("\n-----------------------------------------------\n"); */
    err = wabi_vm_reduce(vm);
    if(! err) continue;
    if(err == wabi_error_nomem) {
      wabi_vm_collect(vm);
      continue;
    }
    return err;
  } while(1);
  /* } while(reductions); */
  printf("IMPOSSIBLE\n");
  return wabi_error_timeout;
}
