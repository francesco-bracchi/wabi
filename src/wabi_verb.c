#define wabi_verb_c

#include <stddef.h>

#include "wabi_value.h"
#include "wabi_verb.h"
#include "wabi_env.h"
#include "wabi_value.h"


wabi_verb
wabi_verb_builtin_new_raw(wabi_store store, void* cfun)
{
  // todo: verify cfun pointer is less then 2^56
  wabi_verb_builtin res = (wabi_verb_builtin) wabi_store_allocate(store, WABI_VERB_BUILTIN_SIZE);
  if(res) {
    *res = WABI_TAG_BOPERATIVE | ((wabi_word_t) cfun);
    return (wabi_verb) res;
  }
  return NULL;
}


wabi_verb
wabi_verb_derived_new_raw(wabi_store store,
                          wabi_env static_env,
                          wabi_symbol dynamic_env_name,
                          wabi_val arguments,
                          wabi_val body)
{
  wabi_verb_derived res = (wabi_verb_derived) wabi_store_allocate(store, WABI_VERB_DERIVED_SIZE);
  if(res) {
    res->static_env = WABI_TAG_OPERATIVE | ((wabi_word_t) static_env);
    res->dynamic_env_name = (wabi_word_t) dynamic_env_name;
    res->arguments = (wabi_word_t) arguments;
    res->body = (wabi_word_t) body;
  }
  return NULL;
}


static inline wabi_verb
wabi_verb_wrap_builtin_raw(wabi_store store, wabi_verb_builtin verb)
{
  wabi_verb_builtin res = (wabi_verb_builtin) wabi_store_allocate(store, WABI_VERB_BUILTIN_SIZE);
  if(res) {
    *res = *verb;
    *res = *res | WABI_VERB_WRAP_MASK;
    return (wabi_verb) res;
  }
  return NULL;
}

static inline wabi_verb
wabi_verb_wrap_derived_raw(wabi_store store, wabi_verb_derived verb)
{
  wabi_verb_derived res = (wabi_verb_derived) wabi_store_allocate(store, WABI_VERB_DERIVED_SIZE);
  if(res) {
    *res = *verb;
    res->static_env = WABI_VERB_WRAP_MASK | verb->static_env;
    return (wabi_verb) res;
  }
  return NULL;
}

wabi_verb
wabi_verb_wrap_raw(wabi_store store, wabi_verb verb)
{
  return WABI_VERB_IS_BUILTIN(verb)
    ? wabi_verb_wrap_builtin_raw(store, (wabi_verb_builtin) verb)
    : wabi_verb_wrap_derived_raw(store, (wabi_verb_derived) verb);
}



static inline wabi_verb
wabi_verb_unwrap_builtin_raw(wabi_store store, wabi_verb_builtin verb)
{
  wabi_verb_builtin res = (wabi_verb_builtin) wabi_store_allocate(store, WABI_VERB_BUILTIN_SIZE);
  if(res) {
    *res = *verb;
    *res = *res & WABI_VERB_UNWRAP_MASK;
    return (wabi_verb) res;
  }
  return NULL;
}

static inline wabi_verb
wabi_verb_unwrap_derived_raw(wabi_store store, wabi_verb_derived verb)
{
  wabi_verb_derived res = (wabi_verb_derived) wabi_store_allocate(store, WABI_VERB_DERIVED_SIZE);
  if(res) {
    *res = *verb;
    res->static_env = WABI_VERB_UNWRAP_MASK & verb->static_env;
    return (wabi_verb) res;
  }
  return NULL;
}

wabi_verb
wabi_verb_unwrap_raw(wabi_store store, wabi_verb verb)
{
  return WABI_VERB_IS_BUILTIN(verb)
    ? wabi_verb_unwrap_builtin_raw(store, (wabi_verb_builtin) verb)
    : wabi_verb_unwrap_derived_raw(store, (wabi_verb_derived) verb);
}


wabi_verb
wabi_verb_new(wabi_vm vm,
              wabi_env static_env,
              wabi_symbol dynamic_env_name,
              wabi_val arguments,
              wabi_val body)
{
  wabi_verb res = wabi_verb_derived_new_raw(&(vm->store), static_env, dynamic_env_name, arguments, body);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}

wabi_verb
wabi_verb_builtin_new(wabi_vm vm, void* fun)
{
  wabi_verb res = wabi_verb_builtin_new_raw(&(vm->store), fun);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}

wabi_verb
wabi_verb_wrap(wabi_vm vm, wabi_verb verb)
{
  wabi_verb res = wabi_verb_wrap_raw(&(vm->store), verb);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}

wabi_verb
wabi_verb_unwrap(wabi_vm vm, wabi_verb verb)
{
  wabi_verb res = wabi_verb_unwrap_raw(&(vm->store), verb);
  if(res) return res;
  vm->errno = WABI_ERROR_NOMEM;
  return NULL;
}
