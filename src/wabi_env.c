#define wabi_map_c

#include <stddef.h>
#include "wabi_env.h"
#include "wabi_store.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_err.h"


wabi_env
wabi_env_new_raw(wabi_store store, wabi_env prev)
{
  wabi_map data = (wabi_map) wabi_map_empty_raw(store);
  if(! data) {
    return NULL;
  }
  wabi_env res = (wabi_env) wabi_store_allocate(store, WABI_ENV_SIZE);
  if(! res)  {
    return NULL;
  }
  res->prev = (wabi_word_t) prev | WABI_TAG_ENVIRONMENT;
  res->data = (wabi_word_t) data;
  return res;
}


wabi_env
wabi_env_empty(wabi_vm vm)
{
  wabi_env res = wabi_env_new_raw(&(vm->store), NULL);
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return (wabi_val) res;
}


wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev)
{
  wabi_env res;
  res = wabi_env_new_raw(&(vm->store), prev);
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k)
{
  wabi_val v = NULL;
  do {
    v = wabi_map_get_raw((wabi_map) env->data, (wabi_val) k);
    if(v) return v;
    env = (wabi_env) (env->prev & WABI_VALUE_MASK);
  } while(env);
  return NULL;
}


wabi_env
wabi_env_assoc(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v)
{
  wabi_map data = wabi_map_assoc_raw(&(vm->store), (wabi_map) env->data, (wabi_val) k, v);
  if(! data) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  env->data = (wabi_word_t) data;
  return env;
}
