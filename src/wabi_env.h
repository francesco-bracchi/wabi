#ifndef wabi_env_h

#define wabi_env_h

#include <stdint.h>

#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_error.h"


typedef struct wabi_env_struct {
  wabi_word prev;
  wabi_word numE;
  wabi_word maxE;
  wabi_word data;
} wabi_env_t;


typedef wabi_env_t* wabi_env;

// todo introduce a wabi_env_entry struct

#define WABI_ENV_SIZE 4
#define WABI_ENV_INITIAL_SIZE 8
#define WABI_ENV_PAIR_SIZE 2
#define WABI_ENV_ALLOC_SIZE (WABI_ENV_SIZE + WABI_ENV_PAIR_SIZE * WABI_ENV_INITIAL_SIZE)
#define WABI_ENV_LOW_LIMIT 8


static inline wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev) {
  wabi_env res;
  res = (wabi_env) wabi_vm_alloc(vm, WABI_ENV_ALLOC_SIZE);
  if(res) {
    res->prev = (wabi_word) prev;
    res->numE = 0;
    res->maxE = WABI_ENV_INITIAL_SIZE;
    res->data = (wabi_word) ((wabi_word*) res + WABI_ENV_SIZE);
    WABI_SET_TAG(res, wabi_tag_env);
  }
  return res;
}


wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v);


wabi_error_type
wabi_env_builtins(wabi_vm vm, wabi_env env);

wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k);

static inline wabi_env
wabi_env_new(wabi_vm vm)
{
  return wabi_env_extend(vm, NULL);
}


#endif
