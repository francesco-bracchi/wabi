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


int
wabi_vm_collect(wabi_vm vm)
{
  wabi_store store;
  store = &(vm->store);

  if(wabi_store_collect_prepare(store)) {
    vm->control = wabi_store_copy_val(store, vm->control);
    vm->env = wabi_store_copy_val(store, vm->env);
    vm->continuation = wabi_store_copy_val(store, vm->continuation);
    return wabi_store_collect(store);
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
  vm->errno = wabi_error_none;
  vm->errval = NULL;
  vm->fuel = 100000;
  if(wabi_store_init(&(vm->store), store_size)) {
    return 1;
  }
  return 0;
}


void
wabi_vm_destroy(wabi_vm vm)
{
  wabi_store_destroy(&(vm->store));
  // free(vm->store);
}


wabi_val
wabi_vm_reverse(wabi_vm vm,
                wabi_val done,
                wabi_val res)
{
  while(*done != wabi_val_nil) {
    res = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) done), res);
    if(! res) return NULL;
    done = wabi_cdr((wabi_pair) done);
  }
  return res;
}


int
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

static inline void
wabi_vm_push_eval(wabi_vm vm, wabi_env env)
{
  vm->continuation = (wabi_val) wabi_cont_eval_new(vm, env, (wabi_cont) vm->continuation);
}

static inline void
wabi_vm_push_apply(wabi_vm vm, wabi_env env, wabi_val expr)
{
  vm->continuation = (wabi_val) wabi_cont_apply_new(vm, env, expr, (wabi_cont) vm->continuation);
}

static inline void
wabi_vm_push_eval_more(wabi_vm vm, wabi_env env, wabi_val as, wabi_val xs)
{
  vm->continuation = (wabi_val) wabi_cont_eval_more_new(vm, env, as, xs, (wabi_cont) vm->continuation);
}

static inline void
wabi_vm_push_call(wabi_vm vm, wabi_env env, wabi_val fun)
{
  vm->continuation = (wabi_val) wabi_cont_call_new(vm, env, fun, (wabi_cont) vm->continuation);
}

static inline void
wabi_vm_push_prog(wabi_vm vm, wabi_env env, wabi_val es)
{
  vm->continuation = (wabi_val) wabi_cont_prog_new(vm, env, es, (wabi_cont) vm->continuation);
}

static inline wabi_cont
wabi_vm_pop(wabi_vm vm)
{
  wabi_cont res = (wabi_cont) vm->continuation;
  vm->continuation = (wabi_val) WABI_WORD_VAL(res->prev);
  return res;
}

wabi_vm_result
wabi_vm_run(wabi_vm vm)
{
  // todo: reduce registers?
  wabi_cont cont;
  wabi_val ctrl, xs, as, nil, cs;
  wabi_env e1;
  wabi_combiner c0;
  wabi_size counter;
  wabi_error_type err;

  if(!wabi_vm_has_rooms(vm, 1))
    return wabi_vm_result_error;

  nil = wabi_vm_alloc(vm, 1);
  *nil = wabi_val_nil;
  counter = 0;

  do {

#ifdef WABI_VM_DEBUG
    printf("reducts:  %lu\n", counter);
    printf("control: ");
    wabi_pr(vm->control);
    if(vm->env) {
      printf("\nenv:     ");
      wabi_pr((wabi_val) vm->env);
    }
    printf("\ncont:    ");
    wabi_pr((wabi_val) vm->continuation);

    printf("\n\n");
#endif

    ctrl = vm->control;
    cont = wabi_vm_pop(vm);

    if(vm->errno) {
      return wabi_vm_result_error;
    }
    switch(WABI_TAG(cont)) {
    case wabi_tag_cont_eval:
      vm->env = (wabi_val) ((wabi_cont_eval) cont)->env;

      if(WABI_IS(wabi_tag_pair, vm->control)) {
        /* control: (f . as) */
        /* stack: ((eval e0) . s) */
        /* -------------------------------------- */
        /* control: f */
        /* stack: ((eval e0) (apply e0 as) . s) */
        wabi_vm_push_apply(vm, (wabi_env) vm->env, wabi_cdr((wabi_pair) ctrl));
        wabi_vm_push_eval(vm, (wabi_env) vm->env);
        vm->control = wabi_car((wabi_pair) ctrl);
        break;
      }
      if(WABI_IS(wabi_tag_symbol, ctrl)) {
        /* control: c when (sym? c) */
        /* stack: ((eval e0) . s) */
        /* -------------------------------------- */
        /* control: (lookup c e0) */
        /* stack s */
        cs = wabi_env_lookup((wabi_env) vm->env, (wabi_symbol) ctrl);

        if(cs) {
          vm->control = cs;
          break;
        }
        vm->errno = wabi_error_unbound_name;
        return wabi_vm_result_error;
      }
      /* control: c */
      /* stack: ((eval e0) . s) */
      /* -------------------------------------- */
      /* control: c */
      /* stack s */
      break;
    case wabi_tag_cont_apply:
      as = (wabi_val) ((wabi_cont_apply) cont)->args;
      vm->env = (wabi_val) ((wabi_cont_apply) cont)->env;

      wabi_vm_push_call(vm, (wabi_env) vm->env, ctrl);

      if(wabi_combiner_is_applicative((wabi_combiner) ctrl) && WABI_IS(wabi_tag_pair, as)) {
        /* control: c when (app? c) */
        /* stack: ((apply e0 (a . as)) . s) */
        /* -------------------------------------- */
        /* control: a */
        /* stack: ((eval-more e0 as nil) (call e0 c) . s) */

        // TODO: optimize the case of single argument?
        wabi_vm_push_eval_more(vm, (wabi_env) vm->env, wabi_cdr((wabi_pair) as), (wabi_val) nil);
        wabi_vm_push_eval(vm, (wabi_env) vm->env);
        vm->control = wabi_car((wabi_pair) as);
        break;
      }
      /* control: c */
      /* stack: ((apply e0 nil) . s) */
      /* -------------------------------------- */
      /* control: nil */
      /* stack: ((call e0 c) . s) */
      vm->control = as;
      break;
    case wabi_tag_cont_eval_more:
      vm->env = (wabi_val) ((wabi_cont_eval_more) cont)->env;
      as = (wabi_val) ((wabi_cont_eval_more) cont)->data;
      xs = (wabi_val) ((wabi_cont_eval_more) cont)->done;

      if(WABI_IS(wabi_tag_pair, as)) {
        /* control: x */
        /* stack: ((eval-more e0 (a . as) xs) . s) */
        /* -------------------------------------- */
        /* control: a */
        /* stack: ((eval e0) (eval-more e0 as (cons x xs)) . s) */
        wabi_vm_push_eval_more(vm, (wabi_env) vm->env, wabi_cdr((wabi_pair) as), (wabi_val) wabi_cons(vm, ctrl, xs));
        wabi_vm_push_eval(vm, (wabi_env) vm->env);
        vm->control = wabi_car((wabi_pair) as);
        break;
      }
      /* control: x */
      /* stack: ((eval-more e0 nil xs) . s) */
      /* -------------------------------------- */
      /* control: (reverse (cons x xs)) */
      /* stack: s */
      vm->control = wabi_vm_reverse(vm, xs, (wabi_val) wabi_cons(vm, ctrl, nil));
      break;
    case wabi_tag_cont_call:
      c0 = (wabi_combiner) ((wabi_cont_call) cont)->combiner;
      if(WABI_IS(wabi_tag_oper, c0) || WABI_IS(wabi_tag_app, c0)) {
        /* control: as */
        /* stack: ((call e0 (fx e1 ex ps b)) . s) */
        /* -------------------------------------- */
        /* control b */
        /* stack: ((eval (bind ex e0 ps as)) . s) */
        e1 = wabi_env_extend(vm, (wabi_env) ((wabi_cont_eval_more) cont)->env);


        err = wabi_vm_bind(vm, e1, (wabi_val) vm->env, (wabi_val) ((wabi_combiner_derived) c0)->caller_env_name);
        if(err == wabi_error_nomem) {
          vm->continuation = (wabi_val) cont;
          if(wabi_vm_collect(vm)) break;
        }
        if(err) {
          vm->errno = err;
          return wabi_vm_result_error;
        }

        err = wabi_vm_bind(vm, e1, ctrl, (wabi_val) ((wabi_combiner_derived) c0)->parameters);
        if(err == wabi_error_nomem) {
          vm->continuation = (wabi_val) cont;
          if(wabi_vm_collect(vm)) break;
        }
        if(err) {
          vm->errno = err;
          return wabi_vm_result_error;
        }
        wabi_vm_push_eval(vm, e1);
        vm->control = (wabi_val) ((wabi_combiner_derived) c0)->body;
        break;
      }
      /* control: as */
      /* stack: ((call e0 fx) . s) when (builtin? fx) */
      /* -------------------------------------- */
      /* control (funcall fx control store env)  */
      /* stack: s */

      ((wabi_builtin_fun) (WABI_WORD_VAL(((wabi_combiner_builtin) c0)->c_ptr)))(vm, (wabi_env) vm->env);
      if(vm->errno == wabi_error_nomem) {
        vm->errno = wabi_error_none;
        vm->continuation = (wabi_val) cont;
        if(!wabi_vm_collect(vm)) break;
        return wabi_vm_result_error;
      }
      break;
    case wabi_tag_cont_sel:
      vm->env = (wabi_val) ((wabi_cont_sel) cont)->env;
      wabi_vm_push_eval(vm, (wabi_env) vm->env);
      if(*ctrl == wabi_val_nil || *ctrl == wabi_val_false) {
        /* control: false | nil */
        /* stack: ((sel e0 l r) . s) */
        /* -------------------------------------- */
        /* control: r */
        /* stack: ((eval e0) . s) */
        vm->control = (wabi_val) ((wabi_cont_sel) cont)->right;
        break;
      }
      /* control: _ */
      /* stack: ((sel e0 l r) . s) */
      /* -------------------------------------- */
      /* control: l */
      /* stack: ((eval e0) . s) */
      vm->control = (wabi_val) ((wabi_cont_sel) cont)->left;
      break;
    case wabi_tag_cont_def:
      /* control: as */
      /* stack: ((def e0 sym) . s) */
      /* -------------------------------------- */
      /* control: as */
      /* stack: s */
      vm->env = (wabi_val) ((wabi_cont_def) cont)->env;
      vm->errno = wabi_vm_bind(vm, (wabi_env) vm->env, ctrl, (wabi_val) ((wabi_cont_def) cont)->pattern);
      if(vm->errno) return wabi_vm_result_error;
      break;
    case wabi_tag_cont_prog:
      if(WABI_IS(wabi_tag_pair, ((wabi_cont_prog) cont)->expressions)) {
        /* control: x0 */
        /* stack: ((prog e0 (x . xs)) . s) */
        /* -------------------------------------- */
        /* control: x */
        /* stack: ((prog e0 xs) . s) */
        vm->env = (wabi_val) ((wabi_cont_prog) cont)->env;
        cs = (wabi_val) ((wabi_cont_prog) cont)->expressions;
        wabi_vm_push_prog(vm, (wabi_env) ((wabi_cont_prog) cont)->env, wabi_cdr((wabi_pair) cs));
        vm->control = wabi_car((wabi_pair) cs);
        break;
      }

      if(*((wabi_word*) (((wabi_cont_prog) cont)->expressions)) == wabi_val_nil) {
        /* control: x0 */
        /* stack: ((prog e0 nil) . s) */
        /* -------------------------------------- */
        /* control: x0 */
        /* stack: s */
        wabi_vm_push_eval(vm, (wabi_env) ((wabi_cont_prog) cont)->env);
        break;
      }
      vm->errno = wabi_error_type_mismatch;
      return wabi_vm_result_error;
    }
    counter++;
  } while (vm->continuation);
  return wabi_vm_result_done;
}
