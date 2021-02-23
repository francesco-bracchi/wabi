#ifndef wabi_list_h

#define wabi_list_h

#include "wabi_value.h"
#include "wabi_atom.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef struct wabi_pair_struct
{
  wabi_word cdr;
  wabi_word car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

#define WABI_PAIR_SIZE wabi_sizeof(wabi_pair_t)


static inline wabi_pair
wabi_cons(const wabi_vm vm,
          const wabi_val car,
          const wabi_val cdr)
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
wabi_pair_hash(const wabi_hash_state state,
               const wabi_pair pair) {
  wabi_hash_step(state, "P", 1);
  wabi_hash_val(state, wabi_car(pair));
  wabi_hash_val(state, wabi_cdr(pair));
}


void
wabi_list_builtins(const wabi_vm vm, const wabi_env env);


static inline int
wabi_is_pair(const wabi_val v)
{
  return WABI_IS(wabi_tag_pair, v);
}


static inline wabi_size
wabi_list_length(const wabi_vm vm, const wabi_val v0)
{
  wabi_val v;
  wabi_size l;
  v = v0;
  l = 0;
  for (;;) {
    if(wabi_atom_is_empty(vm, v)) {
      return l;
    }
    if(!wabi_is_pair(v)) {
      return -1;
    }
    v = wabi_cdr((wabi_pair) v);
    l++;
  }
}

static inline int
wabi_is_list(const wabi_vm vm, const wabi_val v0)
{
  wabi_val v;

  v = v0;
  for (;;) {
    if(wabi_atom_is_empty(vm, v)) {
      return 1;
    }
    if(!wabi_is_pair(v)) {
      return 0;
    }
    v = wabi_cdr((wabi_pair) v);
  }
}

#endif
