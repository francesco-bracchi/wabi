/* #define wabi_vm_c */

/* #include "wabi_vm.h" */
/* #include "wabi_store.h" */
/* #include "wabi_system.h" */

/* #define CONTROL ctrl */
/* #define STACK (store.stack) */
/* #define HEAP (store.heap) */

/* #define STACK_OVERFLOW(size) (HEAP + (size) >= STACK) */
/* #define ALLOC(size) STACK_OVERFLOW(size) ? 0 : HEAP =+ (size) */
/* #define SWAP swap */

/* /\** */
/*  * State steps: */
/*  * CONTROL | STACK  --> NEW_CONTROL | NEW STACK */
/*  * */
/*  * ## EVAL */
/*  * (f . as)         | (EVAL(e0) . s)       --> f             | (EVAL(e0) APPLY(AS) . s) */
/*  * c when (sym? c)  | (EVAL(e0). s)        --> LOOKUP(e0, c) | s */
/*  * c                | (EVAL(e0) . s)       --> c | s */
/*  * */
/*  * ## APPLY */
/*  * c when (oper? c) | (APPLY(e0, as) . s)      --> as | (CALL(e0, c) . s) */
/*  * c when (app? c)  | (APPLY(e0, as) . s)      --> as | (EVAL_ALL(e0) CALL(e0, C) . s) */
/*  * */
/*  * ## CALL */

/*  * as               | (CALL(e0, (fx e1 ex ps b)) . s)  --> b | (EVAL(ext(e1, bind(ps, as), bin(ex e0))) . s) */
/*  * as               | (CALL(e0, bt) . s)               --> BTCALL(bt) */

/*  * ## COND */
/*  * false            | (COND(e0, l, r)  . s)     --> r | (EVAL(e0) . s) */
/*  * nil              | (COND(e0, l, r)  . s)     --> r | (EVAL(e0) . s) */
/*  * _                | (COND(e0, l, r)  . s)     --> l | (EVAL(e0) . s) */
/*  * */
/*  * ## SEQ */
/*  * x                | SEQ(e0, (y . ys))             --> y | (EVAL(e0), SEQ(e0, ys))) */
/*  * x                | SEQ(e0, as) when (nil? as)    -->  x | s */
/*  *\/ */


/* int */
/* wabi_vm_run(wabi_system sys, */
/*             wabi_word* ctrl, */
/*             wabi_env env) */
/* { */
/*   wabi_store_t store; */
/*   wabi_word* error = NULL; */

/*   if(!wabi_store_init(&store, sys->config.store_initial_size)) { */
/*     return 0; */
/*   } */

/*   PUSH_EVAL(env); */
/*   do { */
/*     switch(WABI_TAG(STACK)) { */
/*     case WABI_EVAL: */
/*       switch(WABI_TAG(CONTROL)) { */
/*       case WABI_PAIR: */
/*         WABI_POP; */
/*         WABI_PUSH_APPLY(WABI_CDR(CONTROL)); */
/*         WABI_PUSH_EVAL; */
/*         CONTROL = WABI_CAR(CONTROL); */
/*         break; */
/*       case WABI_SYMBOL: */
/*         CONTROL=WABI_LOOKUP(ENV, CONTROL); */
/*         WABI_POP; */
/*         break; */
/*       default: */
/*         WABI_POP; */
/*       } */
/*       break; */
/*     case WABI_APPLY: */
/*       if(WABI_APPLICATIVE(CONTROL)) { */
/*         swap = STACK; */
/*         STACK_POP; */
/*         WABI_PUSH_CALL(CONTROL); */
/*         WABI_PUSH_EVAL_ALL; */
/*         CONTROL = WABI_APPLY_ARGUMENTS(swap); */
/*       } else { */
/*         swap = STACK; */
/*         STACK_POP; */
/*         WABI_PUSH_CALL(CONTROL); */
/*         CONTROL = WABI_APPLY_ARGUMENTS(swap); */
/*       } */
/*       break; */
/*     case WABI_CALL: */
/*       swap = WABI_CALL_FUNCTION(STACK); */
/*       if (WABI_BUILTIN?(swap)) { */
/*         BTCALL(swap, store); */
/*       } else { */
/*         WABI_PUSH_EVAL(WABI_ENV_EXTEND(WABI_OPERATIVE_ENV(swap), */
/*                                        WABI_BIND(WABI_OPERATIVE_PARAMETERS(swap), CONTROL), */
/*                                        WABI_BIND(WABI_OPERATIVE_DYNAMIC_ENV_NAME(swap), WABI_ENV))); */
/*         CONTROL = WABI_OPERATIVE_BODY(swap); */

/*       } */
/*     } */
/*     fuel--; */
/*   } while(fuel && !error && STACK < store.limit); */

/*   if(! fuel) return 3; */
/*   if(error) return 2; */
/*   if(store.stack == limit) return 1; */
/* } */
