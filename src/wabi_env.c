#define wabi_map_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"


wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev)
{
  wabi_map data = (wabi_map) wabi_map_empty(vm);
  wabi_env res = (wabi_env) wabi_vm_alloc(vm, 2);
  res->prev = (wabi_word) prev;
  res->data = (wabi_word) data;
  WABI_SET_TAG(res, wabi_tag_env);
  return res;;
}

/*** INLINE? ***/
wabi_env
wabi_env_new(wabi_vm vm)
{
  wabi_env res = (wabi_env) wabi_env_extend(vm, NULL);
  WABI_SET_TAG(res, wabi_tag_env);
  return res;
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

/*** OPTIONAL OVERWRITE ***/
int
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v)
{
  wabi_map data = wabi_map_assoc(vm, (wabi_map) env->data, (wabi_val) k, v);
  if(data) {
    env->data = (wabi_word) data;
    return 1;
  }
  return 0;
}
