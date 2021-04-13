/**
 * Environments
 *
 * Environments are first class objects, right now the only way to get them is via
 * fexpressions, but I'm exptecting to have something along the lines `empty-env`
 * `std-env`.
 *
 * Environments are one of the few data types that are not structural, i.e. can be
 * modified, but only in specific ways (i.e. symbols can only be added, and never
 * modified (this will not be completely true in the future but we will see).
 *
 * Environments are similar to vocabularies, in the sense that they map symbols
 * to values.
 *
 * A new environment is created via the `extend` action.
 * The new environment can shadow definitions given in the original environment.
 *
 * When a funciton is called, for example, the body is evaluated in the context
 * of an environment that is the extension of the static env, and the binding
 * operation is run on formal parameters.
 *
 * TODO:
 * 1. merge multiple environmnets
 * 2. more environment introspection
 */

#define wabi_env_c

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_map.h"
#include "wabi_error.h"
#include "wabi_builtin.h"
#include "wabi_atom.h"

static wabi_size rnd = 0;

static inline wabi_word
wabi_env_uid(wabi_env env)
{
  static wabi_word wabi_env_cnt = 0;
  return ((wabi_word) env) ^ (++wabi_env_cnt);
}

wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev)
{
  wabi_env res;
  res = (wabi_env) wabi_vm_alloc(vm, WABI_ENV_ALLOC_SIZE);
  if(vm->ert) return NULL;

  res->prev = (wabi_word) prev;
  res->uid = wabi_env_uid(res);
  res->numE = 0;
  res->maxE = WABI_ENV_INITIAL_SIZE;
  res->data = (wabi_word) ((wabi_word*) res + WABI_ENV_SIZE);
  WABI_SET_TAG(res, wabi_tag_env);
  wabi_env_reset(res);
  return res;
}


static inline void
wabi_env_copy_from(const wabi_env env,
                   const wabi_env_pair old_data,
                   const wabi_word old_size)
{
  int j;
  wabi_env_pair p;
  for (int j = 0; j < old_size; j++) {
    p = old_data + j;
    if(p->key) wabi_env_set_raw(env, p);
  }
}

void
wabi_env_set_expand(const wabi_vm vm,
                    const wabi_env env)
{
  wabi_word old_size, old_data, new_size, new_data;
  old_size = env->maxE;
  old_data = env->data;
  new_size = env->maxE <= 0 ? WABI_ENV_INITIAL_SIZE : env->numE * 2;
  new_data = (wabi_word) wabi_vm_alloc(vm, new_size * WABI_ENV_PAIR_SIZE);
  if(vm->ert) return;

  env->data = new_data;
  env->maxE = new_size;
  wabi_env_reset(env);
  wabi_env_copy_from(env, (wabi_env_pair) old_data, old_size);
}

static inline void
wabi_env_actually_set(const wabi_env env,
                      const wabi_val k,
                      const wabi_val v)
{
  wabi_env_pair_t p;
  p.key = (wabi_word) k;
  p.val = (wabi_word) v;
  wabi_env_set_raw(env, &p);
  env->numE++;
}

wabi_val
wabi_env_lookup_local(const wabi_env env, const wabi_val k)
{
  wabi_env_pair p;
  wabi_env_pair data;
  wabi_word delta;

  data = (wabi_env_pair) env->data;
  delta = wabi_env_hash((wabi_symbol) k) % env->maxE;

  for(;;) {
    p = data + delta;
    if((wabi_val) p->key == k) {
      return (wabi_val) p->val;
    }
    if (p->key == 0) {
      return NULL;
    }
    delta = (delta + 1) % env->maxE;
  }
}

wabi_val
wabi_env_lookup(const wabi_env env0,
                const wabi_val k)
{
  wabi_env env;
  wabi_val res;

  env = env0;
  do {
    res = wabi_env_lookup_local(env, k);
    if(res) return res;
    env = (wabi_env) WABI_WORD_VAL(env->prev);
  } while(env);
  return NULL;
}


void
wabi_env_set(const wabi_vm vm,
             const wabi_env env,
             const wabi_val k,
             const wabi_val v)
{
  // todo raise exception when the key is found in actually set
  if(wabi_env_lookup_local(env, k)) {
    vm->ert = wabi_error_already_defined;
    return;
  }
  if(env->numE > env->maxE * WABI_ENV_FILL_RATIO) {
    wabi_env_set_expand(vm, env);
    if(vm->ert) return;
  }
  wabi_env_actually_set(env, k, v);
}
