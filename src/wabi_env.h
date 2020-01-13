#ifndef wabi_env_h

#define wabi_env_h

#include <stdint.h>

#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_error.h"
#include <stdlib.h>

typedef struct wabi_env_struct {
  wabi_word prev;
  wabi_word numE;
  wabi_word maxE;
  wabi_word data;
} wabi_env_t;


typedef wabi_env_t* wabi_env;


#define WABI_ENV_SIZE 4
#define WABI_ENV_INITIAL_SIZE 8
#define WABI_ENV_PAIR_SIZE 2
#define WABI_ENV_ALLOC_SIZE (WABI_ENV_SIZE + WABI_ENV_PAIR_SIZE * WABI_ENV_INITIAL_SIZE)
#define WABI_ENV_LOW_LIMIT 8


wabi_env
wabi_env_extend(wabi_vm vm, wabi_env prev);


wabi_error_type
wabi_env_set(wabi_vm vm, wabi_env env, wabi_symbol k, wabi_val v);


wabi_error_type
wabi_env_builtins(wabi_vm vm, wabi_env env);


static inline wabi_env
wabi_env_new(wabi_vm vm)
{
  return wabi_env_extend(vm, NULL);
}


static inline wabi_val
wabi_env_lookup(wabi_env env, wabi_symbol k)
{
  wabi_size j, l;
  wabi_symbol k0;
  wabi_val res;
  wabi_word sk, sv;

  do {
    for(j = 0; j < env->numE; j++) {
      k0 = (wabi_val) *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE);
      if(k0 == k) {
        res = (wabi_val) (wabi_val) *((wabi_word*) env->data + 1 + WABI_ENV_PAIR_SIZE * j);
        if (j >= WABI_ENV_LOW_LIMIT) {
          // tricky
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
      /* if(wabi_cmp(k0, k) == 0) { */
      /*   return (wabi_val) (wabi_val) *((wabi_word*) env->data + 1 + WABI_ENV_PAIR_SIZE * j); */
      /* } */
    }
    env = (wabi_env) WABI_WORD_VAL(env->prev);
  } while(env);
  return NULL;
}

#endif
