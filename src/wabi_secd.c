
#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_pair.h"
#include "wabi_store.h"
#include "wabi_combiner.h"
#include "wabi_env.h"

#define GET_ADDRESS ((wabi_val) (*((wabi_word_t*) c) & WABI_VALUE_MASK))

#define TOP WABI_CAR(s)

#define PUSH(v) (s = wabi_cons(store, s, v))

#define POP (s = WABI_CDR(s));

#define DUMP \
  x = wabi_store_allocate(store, WABI_FRAME_SIZE); \
  x->p = d; x->s = s; x->e = e; x->c = c; \
  d = x;

#define IP(n) c+=n

#define IP1 IP(1)

#define STORE_ALLOC(size) ((sa + (size) < sl) ? sa =+ size : 0)


#define ALLOC \

int
wabi_secd_init (wabi_secd vm)
{
  vm->store = wabi
}


wabi_secd_run(wabi_secd vm)
{
  register wabi_val s = vm->stack;
  register wabi_val e = vm->environment;
  register char* c  = vm->control;
  register wabi_val d = vm->dump;
  register unsigned int e = WABI_ERROR_NONE;
  register unsigned int r = 0;
  register wabi_word_t sa = vm->store->alloc;
  register wabi_word_t sl = vm->store->limit;


  wabi_store store = vm->store;
  wabi_val x;

  do {
    switch(*c) {
    case wabi_word_push:
      PUSH(GET_ADDRESS);
      IP(4);
      break;
    case wabi_word_alloc:
      x = ALLOC(TOP);
      if (x) {
        POP;
        PUSH(x);
        break;
      }
      wabi_store_collect(store);
      sa = store->alloc;
      sb = store->limit;
      x = ALLOC(TOP);
      if(x) {
        POP;
        PUSH(x);
        break;
      }
      e = WABI_ERROR_NOMEM;
      break;

    case wabi_word_eval:
      x = TOP;
      POP;
      switch(wabi_val_type(x)) {
      case WABI_TAG_SYMBOL:
        PUSH(wabi_env_lookup(e, x));
        IP1;
        break;
      case WABI_TAG_VAR:
        PUSH(WABI_VAR_VALUE(x));
        IP1;
        break;
      case WABI_TAG_PAIR:
        DUMP;
        PUSH(WABI_CDR(x));
        PUSH(WABI_CAR(x));
        c = APPLY;
      }
      break;
    }
  } while(true);
}
