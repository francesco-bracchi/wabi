#define wabi_map_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_store.h"
#include "wabi_symbol.h"
#include "wabi_map.h"


wabi_env
wabi_env_extend(wabi_store store, wabi_env prev)
{
  wabi_map data = (wabi_map) wabi_map_empty(store);
  if(data) {
    wabi_env res = (wabi_env) wabi_store_heap_alloc(store, WABI_ENV_SIZE);
    if(res) {
      res->prev = (wabi_word) prev;
      res->data = (wabi_word) data;
      WABI_SET_TAG(res, wabi_tag_env);
      return res;
    }
    return NULL;
  }
  return NULL;
}


wabi_env
wabi_env_new(wabi_store store)
{
  wabi_env res = (wabi_env) wabi_env_extend(store, NULL);
  if(res) {
    WABI_SET_TAG(res, wabi_tag_env);
    return res;
  }
  return NULL;
}


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k)
{
  wabi_val v = NULL;
  do {
    v = wabi_map_get((wabi_map) env->data, (wabi_val) k);
    if(v) return v;
    env = (wabi_env) WABI_WORD_VAL(env->prev);

  } while(env);
  return NULL;
}


wabi_env
wabi_env_set(wabi_store store, wabi_env env, wabi_symbol k, wabi_val v)
{
  wabi_map data = wabi_map_assoc(store, (wabi_map) env->data, (wabi_val) k, v);
  if(data) {
    env->data = (wabi_word) data;
    return env;
  }
  return NULL;
}
