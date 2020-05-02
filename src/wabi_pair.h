#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_collect.h"
#include "wabi_env.h"
#include "wabi_cmp.h"

typedef struct wabi_pair_struct
{
  wabi_word cdr;
  wabi_word car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

#define WABI_PAIR_SIZE wabi_sizeof(wabi_pair_t)

/** CY775VF */
/** CY775FV */

static inline wabi_pair
wabi_cons(wabi_vm vm, wabi_val car, wabi_val cdr)
{
  wabi_pair pair = (wabi_pair) wabi_vm_alloc(vm, WABI_PAIR_SIZE);
  if(pair) {
    pair->car = (wabi_word) car;
    pair->cdr = (wabi_word) cdr;
    WABI_SET_TAG(pair, wabi_tag_pair);
  }
  return pair;
}


static inline wabi_val
wabi_car(wabi_pair pair)
{
  return (wabi_val) pair->car;
}


static inline wabi_val
wabi_cdr(wabi_pair pair)
{
  return (wabi_val) WABI_WORD_VAL(pair->cdr);
}

static inline void
wabi_pair_copy_val(wabi_vm vm, wabi_pair pair)
{
  wabi_copy_val_size(vm, (wabi_val) pair, WABI_PAIR_SIZE);
}

static inline void
wabi_pair_collect_val(wabi_vm vm, wabi_pair pair)
{
  wabi_collect_val_size(vm, (wabi_val) pair, WABI_PAIR_SIZE);
}

static inline void
wabi_pair_hash(wabi_hash_state state, wabi_pair pair) {
  wabi_hash_step(state, "P", 1);
  wabi_hash_val(state, wabi_car(pair));
  wabi_hash_val(state, wabi_cdr(pair));
}


static inline int
wabi_pair_cmp(wabi_pair left, wabi_pair right)
{
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}

wabi_error_type
wabi_pair_builtins(wabi_vm vm, wabi_env env);

#endif
