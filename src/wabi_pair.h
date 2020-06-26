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


static inline wabi_pair
wabi_cons(const wabi_vm vm, const wabi_val car, const wabi_val cdr)
{
  wabi_pair pair = (wabi_pair) wabi_vm_alloc(vm, WABI_PAIR_SIZE);
  if(vm->ert) return NULL;

  pair->car = (wabi_word) car;
  pair->cdr = (wabi_word) cdr;
  WABI_SET_TAG(pair, wabi_tag_pair);
  return pair;
}


static inline wabi_val
wabi_car(const wabi_pair pair)
{
  return (wabi_val) pair->car;
}


static inline wabi_val
wabi_cdr(const wabi_pair pair)
{
  return (wabi_val) WABI_WORD_VAL(pair->cdr);
}

static inline void
wabi_pair_copy_val(const wabi_vm vm, const wabi_pair pair)
{
  wabi_copy_val_size(vm, (wabi_val) pair, WABI_PAIR_SIZE);
}

static inline void
wabi_pair_collect_val(const wabi_vm vm, const wabi_pair pair)
{
  wabi_collect_val_size(vm, (wabi_val) pair, WABI_PAIR_SIZE);
}

static inline void
wabi_pair_hash(const wabi_hash_state state, const wabi_pair pair) {
  wabi_hash_step(state, "P", 1);
  wabi_hash_val(state, wabi_car(pair));
  wabi_hash_val(state, wabi_cdr(pair));
}


static inline int
wabi_pair_cmp(const wabi_pair left, const wabi_pair right)
{
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}

void
wabi_pair_builtins(const wabi_vm vm, const wabi_env env);

static inline int
wabi_is_pair(const wabi_val v)
{
  return WABI_IS(wabi_tag_pair, v);
}

/* wabi_val */
/* wabi_pair_concat(const wabi_vm vm, const wabi_val l, const wabi_val r); */
#endif
