#define wabi_map_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_cmp.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


wabi_env
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


wabi_env
wabi_env_new(wabi_vm vm)
{
  return wabi_env_extend(vm, NULL);
}


static inline void
wabi_env_actually_set(wabi_env env, wabi_symbol k, wabi_val v)
{
  *(((wabi_val) env->data) + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) k;
  *(((wabi_val) env->data) + 1 + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) v;
  env->numE++;
}


static inline wabi_error_type
wabi_env_set_expand(wabi_vm vm, wabi_env env)
{
  uint32_t new_size;
  wabi_word *new_data;

  new_size = env->numE * 2;
  new_data = (wabi_word*) wabi_vm_alloc(vm, new_size * WABI_ENV_PAIR_SIZE);
  if(new_data) {
    wordcopy(new_data, (wabi_word*) env->data, env->numE * WABI_ENV_PAIR_SIZE);
    env->data = (wabi_word) new_data;
    env->maxE = new_size;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v)
{
  wabi_error_type err;
  if(env->numE >= env->maxE) {
    err = wabi_env_set_expand(vm, env);
    if(err) return err;
  }

  wabi_env_actually_set(env, k, v);
  return wabi_error_none;
}


static inline wabi_error_type
wabi_env_p_bt(wabi_vm vm, wabi_val e0)
{
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = WABI_IS(wabi_tag_env, e0) ? wabi_val_true : wabi_val_false;
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


WABI_BUILTIN_WRAP1(wabi_env_p_builtin, wabi_env_p_bt)


wabi_error_type
wabi_env_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "env?", "wabi:env?", wabi_env_p_builtin);
  return res;
}
