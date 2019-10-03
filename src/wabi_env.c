#define wabi_map_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_error.h"


wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev)
{
  wabi_map data;
  wabi_env res;
  data = (wabi_map) wabi_map_empty(vm);
  if(data) {
    res = (wabi_env) wabi_vm_alloc(vm, 2);
    if(res) {
      res->prev = (wabi_word) prev;
      res->data = (wabi_word) data;
      WABI_SET_TAG(res, wabi_tag_env);
    }
  }
  return res;
}

/*** INLINE? ***/
wabi_env
wabi_env_new(wabi_vm vm)
{
  wabi_env res;
  res = (wabi_env) wabi_env_extend(vm, NULL);
  if(res) {
    WABI_SET_TAG(res, wabi_tag_env);
  }
  return res;
}


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k)
{
  wabi_val v;
  v = NULL;
  do {
    v = wabi_map_get((wabi_map) env->data, (wabi_val) k);
    if(v) return v;
    env = (wabi_env) WABI_WORD_VAL(env->prev);

  } while(env);
  return NULL;
}


/*** OPTIONAL OVERWRITE ***/
wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v)
{
  wabi_map data = wabi_map_assoc(vm, (wabi_map) env->data, (wabi_val) k, v);
  if(data) {
    env->data = (wabi_word) data;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}
