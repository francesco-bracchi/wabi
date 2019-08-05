#ifndef wabi_env_h

#define wabi_env_h

#include "wabi_map.h"
#include "wabi_store.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"

typedef struct wabi_env_struct {
  wabi_word_t prev;
  wabi_word_t data;
} wabi_env_t;

typedef wabi_env_t* wabi_env;

#define WABI_ENV_SIZE 2
#define WABI_ENV_BYTE_SIZE (WABI_ENV_SIZE * 64)

wabi_env
wabi_env_new_raw(wabi_store store, wabi_env prev);


wabi_env
wabi_env_empty(wabi_vm vm);


wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev);


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k);


wabi_env
wabi_env_def(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v);

#endif
