#ifndef wabi_env_h

#define wabi_env_h

#include <stdint.h>

#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_error.h"
#include "wabi_hash.h"

typedef struct wabi_env_struct {
  wabi_word prev;
  wabi_word uid;
  wabi_word numE;
  wabi_word maxE;
  wabi_word data;
} wabi_env_t;


typedef wabi_env_t* wabi_env;

// todo introduce a wabi_env_entry struct

#define WABI_ENV_SIZE wabi_sizeof(wabi_env_t)
#define WABI_ENV_INITIAL_SIZE 8
#define WABI_ENV_PAIR_SIZE 2
#define WABI_ENV_ALLOC_SIZE (WABI_ENV_SIZE + WABI_ENV_PAIR_SIZE * WABI_ENV_INITIAL_SIZE)
#define WABI_ENV_LOW_LIMIT 8

wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev);

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


void
wabi_env_copy_val(wabi_store store, wabi_env env);


void
wabi_env_collect_val(wabi_store store, wabi_env env);


void
wabi_env_hash(wabi_hash_state state, wabi_env env);


int
wabi_env_cmp(wabi_env left, wabi_env right);

#endif
