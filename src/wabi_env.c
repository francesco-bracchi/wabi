#define wabi_map_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_error.h"
#include "wabi_builtin.h"

wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev)
{
  wabi_map data;
  wabi_env res;
  data = (wabi_map) wabi_map_empty(vm);
  if(data) {
    res = (wabi_env) wabi_vm_alloc(vm, WABI_ENV_SIZE);
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



static inline void
wabi_env_builtin_epush_bt(wabi_vm vm, wabi_env env, wabi_val e0)
{
  wabi_map data;
  wabi_env res;
  if(WABI_IS(wabi_tag_env, e0)) {
    if(wabi_vm_has_rooms(vm, WABI_ENV_SIZE + WABI_MAP_SIZE)) {
      data = (wabi_map) wabi_map_empty(vm);
      res = (wabi_env) wabi_vm_alloc(vm, WABI_ENV_SIZE);
      res->prev = (wabi_word) e0;
      res->data = (wabi_word) data;
      WABI_SET_TAG(res, wabi_tag_env);
      vm->control = (wabi_val) res;
      return;
    }
    vm->errno = wabi_error_nomem;
    return;
  }
  vm->errno = wabi_error_type_mismatch;
}

static inline void
wabi_env_p_bt(wabi_vm vm, wabi_env env, wabi_val e0)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = (wabi_val) wabi_vm_alloc(vm, 1);
    *res = WABI_IS(wabi_tag_env, e0) ? wabi_val_true : wabi_val_false;
    vm->control = res;
    return;
  }
  vm->errno = wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_env_p_builtin, wabi_env_p_bt)
WABI_BUILTIN_WRAP1(wabi_env_builtin_epush, wabi_env_builtin_epush_bt)


void
wabi_env_builtins(wabi_vm vm, wabi_env env)
{
  WABI_DEFN(vm, env, "env?", "wabi:env?", wabi_env_p_builtin);
  WABI_DEFN(vm, env, "epush", "wabi:epush", wabi_env_builtin_epush);
}
