#define wabi_map_c

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_cmp.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_error.h"
#include "wabi_builtin.h"

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

  new_size = env->numE <= 0 ? WABI_ENV_INITIAL_SIZE : env->numE * 2;
  new_data = (wabi_word*) wabi_vm_alloc(vm, new_size * WABI_ENV_PAIR_SIZE);
  if(new_data) {
    wordcopy(new_data, (wabi_word*) env->data, env->numE * WABI_ENV_PAIR_SIZE);
    env->data = (wabi_word) new_data;
    env->maxE = new_size;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_val
wabi_env_lookup_local(wabi_env env, wabi_symbol k)
{
  wabi_size j, l;
  wabi_symbol k0;
  wabi_val res;
  wabi_word sk, sv;

  for(j = 0; j < env->numE; j++) {
    k0 = (wabi_val) *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE);
    if(k0 != k) continue;
    res = (wabi_val) (wabi_val) *((wabi_word*) env->data + 1 + WABI_ENV_PAIR_SIZE * j);
    if (j >= WABI_ENV_LOW_LIMIT) {
      // this stuff moves the most recent visited symbols at the first part of the list
      l = rand() % WABI_ENV_LOW_LIMIT;
      sk = *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE);
      sv = *((wabi_word*) env->data + 1 + j * WABI_ENV_PAIR_SIZE);
      *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE) = *((wabi_word*) env->data + l * WABI_ENV_PAIR_SIZE);
      *((wabi_word*) env->data + 1 + j * WABI_ENV_PAIR_SIZE) = *((wabi_word*) env->data + 1 + l * WABI_ENV_PAIR_SIZE);
      *((wabi_word*) env->data + l * WABI_ENV_PAIR_SIZE) = sk;
      *((wabi_word*) env->data + 1 + l * WABI_ENV_PAIR_SIZE) = sv;
    }
    return res;
  }
  return NULL;
}


wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v)
{

  wabi_error_type err;
  if(wabi_env_lookup_local(env, k)) {
    return wabi_error_already_defined;
  }
  if(env->numE >= env->maxE) {
    err = wabi_env_set_expand(vm, env);
    if(err) return err;
  }
  wabi_env_actually_set(env, k, v);
  return wabi_error_none;
}


wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k)
{
  wabi_val res;
  do {
    res = wabi_env_lookup_local(env, k);
    if(res) return res;
    env = (wabi_env) WABI_WORD_VAL(env->prev);
  } while(env);
  return NULL;
}

static inline wabi_error_type
wabi_env_p_bt(wabi_vm vm, wabi_val e0)
{
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = WABI_IS(wabi_tag_env, e0) ? wabi_val_true : wabi_val_false;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

static inline wabi_error_type
wabi_env_extend_bt(wabi_vm vm, wabi_val e0)
{
  wabi_val res;
  if(! WABI_IS(wabi_tag_env, e0))
    return wabi_error_type_mismatch;

  res = (wabi_val) wabi_env_extend(vm, (wabi_env) e0);

  if(! res)
    return wabi_error_nomem;

  vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
  vm->control = res;
  return wabi_error_none;
}

WABI_BUILTIN_WRAP1(wabi_env_p_builtin, wabi_env_p_bt)
WABI_BUILTIN_WRAP1(wabi_env_ext_excl, wabi_env_extend_bt)

wabi_error_type
wabi_env_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "env?", "wabi:env?", wabi_env_p_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, "ext!", "wabi:extend!", wabi_env_ext_excl);
  return res;
}
