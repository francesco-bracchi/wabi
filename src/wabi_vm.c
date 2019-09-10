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

wabi_control
wabi_vm_reverse(wabi_store store,
                wabi_val done,
                wabi_val res)
{
  while(*done != wabi_val_nil) {
    res = (wabi_val) wabi_cons(store, wabi_car((wabi_pair) done), res);
    done = wabi_cdr((wabi_pair) done);
  }
  return res;
}


int
wabi_vm_bind(wabi_store store,
             wabi_env env,
             wabi_val params,
             wabi_val args)
{
  int partial;
  if(WABI_IS(wabi_tag_symbol, params)) {
    wabi_env_set(store, env, (wabi_symbol) params, args);
    return 1;
  }
  if(WABI_IS(wabi_tag_pair, params) && WABI_IS(wabi_tag_pair, args)) {
    partial = wabi_vm_bind(store, env, wabi_car((wabi_pair) params), wabi_car((wabi_pair) args));
    if(!partial) return 0;
    return wabi_vm_bind(store, env, wabi_cdr((wabi_pair) params), wabi_cdr((wabi_pair) args));
  }
  if(wabi_cmp(params, args) == 0) {
    return 1;
  }
  return 0;
}

int
wabi_vm_run(wabi_control control,
            wabi_env env,
            wabi_store store,
            wabi_cont cont)
{
  wabi_cont prev;
  wabi_val xs, as, nil, cs;
  wabi_env e0;
  wabi_combiner c0;

  nil = (wabi_val) wabi_store_heap_alloc(store, 1);
  *nil = wabi_val_nil;

  do {
    prev = (wabi_cont) ((wabi_cont) cont)->prev;
    switch(WABI_TAG(cont)) {
    case wabi_tag_cont_eval:
      if(WABI_IS(wabi_tag_pair, control)) {
        /* control: (f . as) */
        /* stack: ((eval e0) . s) */
        /* -------------------------------------- */
        /* control: f */
        /* stack: ((eval e0) (apply e0 as) . s) */
        cont = wabi_cont_apply_new(store, env, wabi_cdr((wabi_pair) control), prev);
        control = wabi_car((wabi_pair) control);
        continue;
      }
      if(WABI_IS(wabi_tag_symbol, control)) {
        /* control: c when (sym? c) */
        /* stack: ((eval e0) . s) */
        /* -------------------------------------- */
        /* control: (lookup c e0) */
        /* stack s */
        control = wabi_env_lookup(env, (wabi_symbol) control);
        continue;
      }
      /* control: c */
      /* stack: ((eval e0) . s) */
      /* -------------------------------------- */
      /* control: c */
      /* stack s */
      continue;
    case wabi_tag_cont_apply:
      as = (wabi_val) ((wabi_cont_apply) cont)->args;
      e0 = (wabi_env) ((wabi_cont_apply) cont)->env;
      if(WABI_IS(wabi_tag_pair, as)) {
        /* control: c when (app? c) */
        /* stack: ((apply e0 (a . as)) . s) */
        /* -------------------------------------- */
        /* control: a */
        /* stack: ((eval-more e0 as nil) (call e0 c) . s) */
        cont = wabi_cont_eval_more_new(store, env, wabi_cdr((wabi_pair) as), nil, prev);
        control = wabi_car((wabi_pair) as);
        continue;
      }
      /* control: c */
      /* stack: ((apply e0 nil) . s) */
      /* -------------------------------------- */
      /* control: nil */
      /* stack: ((call e0 c) . s) */
      cont = wabi_cont_call_new(store, env, control, prev);
      control = as;
      continue;

    case wabi_tag_cont_eval_more:
      e0 = (wabi_env) ((wabi_cont_eval_more) cont)->env;
      as = (wabi_val) ((wabi_cont_eval_more) cont)->data;
      xs = (wabi_val) ((wabi_cont_eval_more) cont)->done;
      if(WABI_IS(wabi_tag_pair, as)) {
        /* control: x */
        /* stack: ((eval-more e0 (a . as) xs) . s) */
        /* -------------------------------------- */
        /* control: a */
        /* stack: ((eval e0) (eval-more e0 as (cons x xs)) . s) */
        cont = wabi_cont_eval_more_new(store, e0, wabi_cdr((wabi_pair) as), (wabi_val) wabi_cons(store, control, xs), prev);
        cont = wabi_cont_eval_new(store, e0, prev);
        control = wabi_car((wabi_pair) as);
        continue;
      }
      /* control: x */
      /* stack: ((eval-more e0 nil xs) . s) */
      /* -------------------------------------- */
      /* control: (reverse (cons x xs)) */
      /* stack: s */
      control = wabi_vm_reverse(store, xs, (wabi_val) wabi_cons(store, control, nil));
      continue;
    case wabi_tag_cont_call:
      c0 = (wabi_combiner) ((wabi_cont_call) cont)->combiner;
      e0 = (wabi_env) ((wabi_cont_eval_more) cont)->env;

      if(WABI_IS(wabi_tag_oper, c0) || WABI_IS(wabi_tag_app, c0)) {
        /* control: as */
        /* stack: ((call e0 (fx e1 ex ps b)) . s) */
        /* -------------------------------------- */
        /* control b */
        /* stack: ((eval (bind ex e0 ps as)) . s) */
        e0 = wabi_env_extend(store, e0);
        wabi_env_set(store, e0, (wabi_symbol) ((wabi_combiner_derived) c0)->caller_env_name, (wabi_val) env);
        if(wabi_vm_bind(store, e0, (wabi_val) ((wabi_combiner_derived) c0)->parameters, control)) {
          control = (wabi_val) ((wabi_combiner_derived) c0)->body;
          cont = wabi_cont_eval_new(store, e0, prev);
          continue;
        }
        return 2;
      }
      /* control: as */
      /* stack: ((call e0 fx) . s) when (builtin? fx) */
      /* -------------------------------------- */
      /* control (funcall fx control store env)  */
      /* stack: s */
      ((wabi_builtin_fun) ((wabi_combiner_builtin) c0)->c_ptr)(&control, &store, &env, &cont);
      cont = prev;
      continue;
    case wabi_tag_cont_sel:
      e0 = (wabi_env) ((wabi_cont_sel) cont)->env;
      cont = wabi_cont_eval_new(store, e0, prev);
      if(*control == wabi_val_nil || *control == wabi_val_false) {
        /* control: false | nil */
        /* stack: ((sel e0 l r) . s) */
        /* -------------------------------------- */
        /* control: r */
        /* stack: ((eval e0) . s) */
        control = (wabi_val) ((wabi_cont_sel) cont)->right;
        continue;
      }
      /* control: _ */
      /* stack: ((sel e0 l r) . s) */
      /* -------------------------------------- */
      /* control: l */
      /* stack: ((eval e0) . s) */
      control = (wabi_val) ((wabi_cont_sel) cont)->left;
      continue;
    case wabi_tag_cont_prog:
      e0 = (wabi_env) ((wabi_cont_prog) cont)->env;
      cs = (wabi_val) ((wabi_cont_prog) cont)->controls;
      if (WABI_IS(wabi_tag_pair, cs)) {
        /* control: x0 */
        /* stack: ((prog e0 (x . xs)) . s) */
        /* -------------------------------------- */
        /* control: x */
        /* stack: ((prog e0 xs) . s) */
        cont = wabi_cont_prog_new(store, e0, wabi_cdr((wabi_pair) cs), prev);
        control = wabi_car((wabi_pair) cs);
        continue;
      }
      /* control: x0 */
      /* stack: ((prog e0 nil) . s) */
      /* -------------------------------------- */
      /* control: x0 */
      /* stack: s */
      continue;
    case wabi_tag_cont_def:
      /* control: as */
      /* stack: ((def e0 sym) . s) */
      /* -------------------------------------- */
      /* control: as */
      /* stack: s */
      e0 = (wabi_env) ((wabi_cont_def) cont)->env;
      if(wabi_vm_bind(store, e0, control, (wabi_val) ((wabi_cont_def) cont)->pattern)) {
        prev = cont;
        continue;
      }
      return 1;
    }
  } while (*((wabi_val) cont) != wabi_val_nil);
  return 0;
}


/* int */
/* wabi_vm_eval() */
/* { */
/*   wabi_store_t store; */
/*   wabi_cont cont; */
/*   wabi_env env; */
/*   wabi_val control; */

/*   if(wabi_store_init(&store, 1000000)) { */
/*     env = wabi_env_new(&store); */
/*     wabi_binary bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(&store, "+"); */
/*     wabi_symbol sym = wabi_symbol_new(&store, (wabi_val) bin); */
/*     wabi_combiner btn = wabi_combiner_builtin_new(&store, bin, &wabi_vm_sum); */
/*     wabi_env_set(&store, env, sym, (wabi_val) btn); */
/*     return wabi_vm_run(control, env, &store, cont); */
/*   } */
/*   return 1; */
/* } */
