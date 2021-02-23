#ifndef wabi_env_h

#define wabi_env_h

#include <stdint.h>

#include "wabi_vm.h"
#include "wabi_error.h"
#include "wabi_hash.h"

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
wabi_env_extend(const wabi_vm vm,
                const wabi_env prev);

void
wabi_env_builtins(const wabi_vm vm,
                  const wabi_env env);

static inline wabi_env
wabi_env_new(const wabi_vm vm)
{
  return wabi_env_extend(vm, NULL);
}

int
wabi_env_cmp(const wabi_env left,
             const wabi_env right);

static inline int
wabi_is_env(const wabi_val val) {
  return WABI_IS(wabi_tag_env, val);
}

void
wabi_env_set_expand(const wabi_vm vm,
                    const wabi_env env);
wabi_val
wabi_env_lookup(const
                wabi_env env0,
                const wabi_val k);

void
wabi_env_set(const wabi_vm vm,
             const wabi_env env,
             const wabi_val k,
             const wabi_val v);
#endif
