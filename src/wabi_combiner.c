#define wabi_combiner_c

#include <stddef.h>

#include "wabi_value.h"
#include "wabi_combiner.h"
#include "wabi_env.h"
#include "wabi_value.h"


wabi_combiner
wabi_combiner_builtin_new_raw(wabi_store store, void* cfun)
{
  // todo: verify cfun pointer is less then 2^56
  wabi_combiner_builtin res = (wabi_combiner_builtin) wabi_store_allocate(store, WABI_COMBINER_BUILTIN_SIZE);
  if(res) {
    *res = WABI_TAG_BUILTIN_OP | ((wabi_word_t) cfun);
    return (wabi_combiner) res;
  }
  return NULL;
}


wabi_combiner
wabi_combiner_derived_new_raw(wabi_store store,
                          wabi_env static_env,
                          wabi_symbol dynamic_env_name,
                          wabi_val arguments,
                          wabi_val body)
{
  wabi_combiner_derived res = (wabi_combiner_derived) wabi_store_allocate(store, WABI_COMBINER_DERIVED_SIZE);
  if(res) {
    res->static_env = WABI_TAG_OPERATIVE | ((wabi_word_t) static_env);
    res->caller_env_name = (wabi_word_t) dynamic_env_name;
    res->arguments = (wabi_word_t) arguments;
    res->body = (wabi_word_t) body;
  }
  return NULL;
}


static inline wabi_combiner
wabi_combiner_wrap_builtin_raw(wabi_store store, wabi_combiner_builtin combiner)
{
  wabi_combiner_builtin res = (wabi_combiner_builtin) wabi_store_allocate(store, WABI_COMBINER_BUILTIN_SIZE);
  if(res) {
    *res = *combiner;
    *res = *res | WABI_COMBINER_WRAP_MASK;
    return (wabi_combiner) res;
  }
  return NULL;
}


static inline wabi_combiner
wabi_combiner_wrap_derived_raw(wabi_store store, wabi_combiner_derived combiner)
{
  wabi_combiner_derived res = (wabi_combiner_derived) wabi_store_allocate(store, WABI_COMBINER_DERIVED_SIZE);
  if(res) {
    *res = *combiner;
    res->static_env = WABI_COMBINER_WRAP_MASK | combiner->static_env;
    return (wabi_combiner) res;
  }
  return NULL;
}


wabi_combiner
wabi_combiner_wrap_raw(wabi_store store, wabi_combiner combiner)
{
  return WABI_COMBINER_IS_BUILTIN(combiner)
    ? wabi_combiner_wrap_builtin_raw(store, (wabi_combiner_builtin) combiner)
    : wabi_combiner_wrap_derived_raw(store, (wabi_combiner_derived) combiner);
}


static inline wabi_combiner
wabi_combiner_unwrap_builtin_raw(wabi_store store, wabi_combiner_builtin combiner)
{
  wabi_combiner_builtin res = (wabi_combiner_builtin) wabi_store_allocate(store, WABI_COMBINER_BUILTIN_SIZE);
  if(res) {
    *res = *combiner;
    *res = *res & WABI_COMBINER_UNWRAP_MASK;
    return (wabi_combiner) res;
  }
  return NULL;
}


static inline wabi_combiner
wabi_combiner_unwrap_derived_raw(wabi_store store, wabi_combiner_derived combiner)
{
  wabi_combiner_derived res = (wabi_combiner_derived) wabi_store_allocate(store, WABI_COMBINER_DERIVED_SIZE);
  if(res) {
    *res = *combiner;
    res->static_env = WABI_COMBINER_UNWRAP_MASK & combiner->static_env;
    return (wabi_combiner) res;
  }
  return NULL;
}


wabi_combiner
wabi_combiner_unwrap_raw(wabi_store store, wabi_combiner combiner)
{
  return WABI_COMBINER_IS_BUILTIN(combiner)
    ? wabi_combiner_unwrap_builtin_raw(store, (wabi_combiner_builtin) combiner)
    : wabi_combiner_unwrap_derived_raw(store, (wabi_combiner_derived) combiner);
}


wabi_combiner
wabi_combiner_new(wabi_vm vm,
              wabi_env static_env,
              wabi_symbol dynamic_env_name,
              wabi_val arguments,
              wabi_val body)
{
  wabi_combiner res = wabi_combiner_derived_new_raw(&(vm->store), static_env, dynamic_env_name, arguments, body);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}


wabi_combiner
wabi_combiner_builtin_new(wabi_vm vm, void* fun)
{
  wabi_combiner res = wabi_combiner_builtin_new_raw(&(vm->store), fun);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}


wabi_combiner
wabi_combiner_wrap(wabi_vm vm, wabi_combiner combiner)
{
  wabi_combiner res = wabi_combiner_wrap_raw(&(vm->store), combiner);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}


wabi_combiner
wabi_combiner_unwrap(wabi_vm vm, wabi_combiner combiner)
{
  wabi_combiner res = wabi_combiner_unwrap_raw(&(vm->store), combiner);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}
