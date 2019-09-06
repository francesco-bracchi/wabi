#define wabi_vm_c

#include <stdint.h>
#include <stddef.h>
#include "wabi_vm.h"
#include "wabi_store.h"
#include "wabi_cont.h"
#include "wabi_system.h"

/**
 * State steps:
 * CONTROL          | STACK                             --> NEW_CONTROL       | NEW STACK

 * (f . as)         | ((eval e0) . s)                   --> f                 | ((eval e0) (apply e0 as) . s)
 * c (sym? c)       | ((eval e0) . s)                   --> (lookup c e0)     | s
 * c                | ((eval e0) . s)                   --> c                 | s

 * c (oper? c)      | ((apply e0 as) . s)               --> as                | ((call e0 c) . s)
 * c (app? c)       | ((apply e0 as) . s)               --> as                | ((eval-all e0) (call e0 c) . s)

 * as               | ((call e0 (fx e1 ex ps b)) . s)   --> b                 | ((eval (ext e1 (bind ex e0) (bind ps as))) . s)
 * as               | ((call e0 bt . s)                 --> (btcall bt as &s) | s
 *
 * false            | ((sel e0 l r) . s)                --> r                 | ((eval e0) . s)
 * nil              | ((sel e0 l r) . s)                --> r                 | ((eval e0) . s)
 * c                | ((sel e0 l r) . s)                --> l                 | ((eval e0) . s)

 * nil              | ((eval-all e0) . s)               --> nil               | s
 * (a . as)         | ((eval-all e0) . s)               --> a                 | ((eval e0) (eval-more e0 as nil) . s)
 * x                | ((eval-more e0 (a . as) xs) . s)  --> a                 | ((eval e0) (eval-more e0 as (x . xs)) . s)
 * x                | ((eval-more e0 nil xs) . s)       --> nil               | ((eval-rev (x . xs)) . s)
 * ys               | ((eval-rev nil) . s)              --> ys                | s
 * ys               | ((eval-rev (x . xs) . s)          --> (x . ys)          | ((eval-rev xs) . s)

 * x                | ((prog e0 nil) . s)                --> x                 | s
 * x                | ((prog e0 (a . as) . s)            --> a                 | ((eval e0) (prog e0 as) . s)
 * x                | ()                                --> SUCCESS(x)
 *
 * ### fuel
 *
 * fuel is decremented only on eval operations?
 */



int
wabi_vm_run(wabi_system sys,
            wabi_val ctrl,
            wabi_val env)
{
  wabi_store_t store;
  wabi_word* error = NULL;
  wabi_cont cont;
  uint64_t fuel = sys->config.fuel;

  if(!wabi_store_init(&store, sys->config.store_initial_size)) {
    return 0;
  }

  wabi_cont_eval_push(&store, env);

  do {
  } while(fuel && !error);

  if(! fuel) return 3;
  if(error) return 2;
}
