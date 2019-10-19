#ifndef wabi_env_h

#define wabi_env_h

#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_error.h"

typedef struct wabi_env_struct {
  wabi_word prev;
  wabi_word data;
} wabi_env_t;

typedef wabi_env_t* wabi_env;

#define WABI_ENV_SIZE 2

wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev);


wabi_env
wabi_env_new(wabi_vm vm);


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k);


wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v);


wabi_error_type
wabi_env_builtins(wabi_vm vm, wabi_env env);
#endif
