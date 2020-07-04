#ifndef wabi_env_h

#define wabi_env_h

#include <stdint.h>

#include "wabi_vm.h"
#include "wabi_error.h"
#include "wabi_hash.h"
#include "wabi_collect.h"

typedef struct wabi_env_struct {
  wabi_word prev;
  wabi_word uid;
  wabi_word numE;
  wabi_word maxE;
  wabi_word data;
} wabi_env_t;


typedef wabi_env_t* wabi_env;

// todo introduce a wabi_env_entry struct

#define WABI_ENV_SIZE wabi_sizeof(wabi_env_t)
#define WABI_ENV_INITIAL_SIZE 8
#define WABI_ENV_PAIR_SIZE 2
#define WABI_ENV_ALLOC_SIZE (WABI_ENV_SIZE + WABI_ENV_PAIR_SIZE * WABI_ENV_INITIAL_SIZE)
#define WABI_ENV_LOW_LIMIT 8

wabi_env
wabi_env_extend(const wabi_vm vm, const wabi_env prev);


void
wabi_env_builtins(const wabi_vm vm, const wabi_env env);


static inline wabi_env
wabi_env_new(const wabi_vm vm)
{
  return wabi_env_extend(vm, NULL);
}


static inline void
wabi_env_copy_val(const wabi_vm vm, const wabi_env env)
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

void
wabi_env_collect_val(const wabi_vm vm, const wabi_env env);



static inline void
wabi_env_hash(wabi_hash_state state, wabi_env env)
{
  wabi_hash_step(state, "E", 1);
  wabi_hash_step(state, (char*) env->uid, WABI_WORD_SIZE);
}


static inline int
wabi_env_cmp(wabi_env left, wabi_env right)
{
  if(left->uid == right->uid) return 0;
  if(left->uid > right->uid) return 1;
  return -1;
}


int
wabi_env_cmp(const wabi_env left, const wabi_env right);


static inline int
wabi_is_env(const wabi_val val) {
  return WABI_IS(wabi_tag_env, val);
}


static wabi_size rnd = 0;

static inline void
wabi_env_actually_set(const wabi_env env,
                      const wabi_val k,
                      const wabi_val v)
{
  *(((wabi_val) env->data) + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) k;
  *(((wabi_val) env->data) + 1 + WABI_ENV_PAIR_SIZE * env->numE) = (wabi_word) v;
  env->numE++;
}


static inline void
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


static inline wabi_val
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


static inline void
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

#endif
