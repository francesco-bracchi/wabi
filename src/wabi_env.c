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
 * 2. more environment manipulation (i.e. undef?)
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
#include "wabi_hash.h"

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
  if(res) {
    res->prev = (wabi_word) prev;
    res->uid = wabi_env_uid(res);
    res->numE = 0;
    res->maxE = WABI_ENV_INITIAL_SIZE;
    res->data = (wabi_word) ((wabi_word*) res + WABI_ENV_SIZE);
    WABI_SET_TAG(res, wabi_tag_env);
  }
  return res;
}

/**
 * Collecting
 */

void
wabi_env_collect_val(wabi_vm vm, wabi_env env)
{
  wabi_size j;
  wabi_word *k, *v;
  if(WABI_WORD_VAL(env->prev)) {
    env->prev = (wabi_word) wabi_copy_val(vm, (wabi_val) WABI_WORD_VAL(env->prev));
  }
  env->maxE = env->numE;
  for(j = 0; j < env->numE; j++) {
    k = ((wabi_val) env->data) + 2 * j;
    v = ((wabi_val) env->data) + 1 + 2 * j;
    *k = (wabi_word) wabi_copy_val(vm, (wabi_val) *k);
    *v = (wabi_word) wabi_copy_val(vm, (wabi_val) *v);
  }
  WABI_SET_TAG(env, wabi_tag_env);
  vm->stor.scan += WABI_ENV_SIZE + env->numE * WABI_ENV_PAIR_SIZE;
}


void
wabi_env_set_expand(const wabi_vm vm,
                    const wabi_env env)
{
  uint32_t new_size;
  wabi_word *new_data;

  new_size = env->numE <= 0 ? WABI_ENV_INITIAL_SIZE : env->numE * 2;
  new_data = (wabi_word*) wabi_vm_alloc(vm, new_size * WABI_ENV_PAIR_SIZE);
  if(vm->ert) return;

  wordcopy(new_data, (wabi_word*) env->data, env->numE * WABI_ENV_PAIR_SIZE);
  env->data = (wabi_word) new_data;
  env->maxE = new_size;
}

static inline void
wabi_env_actually_set(const wabi_env env,
                      const wabi_val k,
                      const wabi_val v)
{
  *(((wabi_val) env->data) + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) k;
  *(((wabi_val) env->data) + 1 + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) v;
  env->numE++;
}

static inline wabi_val
wabi_env_lookup_local(const wabi_env env, const wabi_val k)
{
  wabi_size j, l;
  wabi_val k0, res;
  wabi_word sk, sv;

  for(j = 0; j < env->numE; j++) {
    k0 = (wabi_val) *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE);
    if(k0 != k) continue;
    res = (wabi_val) (wabi_val) *((wabi_word*) env->data + 1 + WABI_ENV_PAIR_SIZE * j);
    if(j >= WABI_ENV_LOW_LIMIT) {
      // this stuff moves the most recent visited symbols at the first part of the list
      // can a better algorithm be devised?
      rnd = l = (rnd+1) % WABI_ENV_LOW_LIMIT;
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

wabi_val
wabi_env_lookup(const
                wabi_env env0,
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
  if(wabi_env_lookup_local(env, k)) {
    vm->ert = wabi_error_already_defined;
    return;
  }
  if(env->numE >= env->maxE) {
    wabi_env_set_expand(vm, env);
    if(vm->ert) return;
  }
  wabi_env_actually_set(env, k, v);
}

void
wabi_env_copy_val(const wabi_vm vm,
                  const wabi_env env)
{
  wabi_size size;
  wabi_word *res;

  res = vm->stor.heap;
  size = env->numE * WABI_ENV_PAIR_SIZE;
  wordcopy(res, (wabi_word*) env, WABI_ENV_SIZE);
  wordcopy(res + WABI_ENV_SIZE, (wabi_word*) env->data, size);
  ((wabi_env)res)->data = (wabi_word) (res + WABI_ENV_SIZE);
  vm->stor.heap += WABI_ENV_SIZE + size;
}

/**
 * Builtins
 */
static void
wabi_env_p_bt(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_env);
}

static void
wabi_env_extend_bt(const wabi_vm vm)
{
  // todo: how about multiple extensions?
  wabi_val ctrl, res;
  wabi_env e0;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  e0 = (wabi_env) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_env((wabi_val) e0)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  res = (wabi_val) wabi_env_extend(vm, e0);
  if(vm->ert) return;

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}

void
wabi_env_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "env?", &wabi_env_p_bt);
  if(vm->ert) return;
  wabi_defn(vm, env, "ext", &wabi_env_extend_bt);
}
