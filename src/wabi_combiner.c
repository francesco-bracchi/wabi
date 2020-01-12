#define wabi_combiner_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_builtin.h"
#include "wabi_error.h"

wabi_combiner
wabi_operator_builtin_new(wabi_vm vm,
                          wabi_binary cname,
                          wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  WABI_SET_TAG(res, wabi_tag_bt_oper);
  return (wabi_combiner) res;
}


wabi_combiner
wabi_application_builtin_new(wabi_vm vm,
                             wabi_binary cname,
                             wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  WABI_SET_TAG(res, wabi_tag_bt_app);
  return (wabi_combiner) res;
}


// TODO: support implicit multiexpression body?
static inline wabi_error_type
wabi_combiner_fx_bt(wabi_vm vm, wabi_val e, wabi_val fs, wabi_val bd)
{
  wabi_combiner_derived res;

  if(!WABI_IS(wabi_tag_symbol, e) && *e != wabi_val_ignore) {
    return wabi_error_type_mismatch;
  }
  res = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
  if(res) {
    res->static_env = (wabi_word) ((wabi_cont_call) vm->continuation)->env;
    res->caller_env_name = (wabi_word) e;
    res->parameters = (wabi_word) fs;
    res->body = (wabi_word) bd;
    WABI_SET_TAG(res, wabi_tag_oper);
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = (wabi_val) res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_combiner_wrap_bt(wabi_vm vm, wabi_val fx)
{
  wabi_val res;

  switch(WABI_TAG(fx)) {
  case wabi_tag_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(res) {
      memcpy(res, fx, sizeof(wabi_combiner_derived_t));
      *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
      WABI_SET_TAG(res, wabi_tag_app);
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  case wabi_tag_bt_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(res) {
      memcpy(res, fx, sizeof(wabi_combiner_builtin_t));
      *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
      WABI_SET_TAG(res, wabi_tag_bt_app);
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_type_mismatch;
}


static inline wabi_error_type
wabi_combiner_unwrap_bt(wabi_vm vm, wabi_val fn)
{
  wabi_val res;

  switch(WABI_TAG(fn)) {
  case wabi_tag_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(res) {
      memcpy(res, fn, sizeof(wabi_combiner_derived_t));
      *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
      WABI_SET_TAG(res, wabi_tag_oper);
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  case wabi_tag_bt_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(res) {
      memcpy(res, fn, sizeof(wabi_combiner_builtin_t));
      *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
      WABI_SET_TAG(res, wabi_tag_bt_oper);
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  default:
    return wabi_error_type_mismatch;
  }
}


static inline wabi_error_type
wabi_combiner_combiner_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

  res = wabi_vm_alloc(vm, 1);
  if(res) {
    // todo optimize in a single bitwise operation?
    switch(WABI_TAG(v)) {
    case wabi_tag_app:
    case wabi_tag_bt_app:
    case wabi_tag_oper:
    case wabi_tag_bt_oper:
      *res = wabi_val_true;
    default:
      *res = wabi_val_false;
    }
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_combiner_applicative_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

    // todo optimize in a single bitwise operation?
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    switch(WABI_TAG(v)) {
    case wabi_tag_app:
    case wabi_tag_bt_app:
      *res = wabi_val_true;
    default:
      *res = wabi_val_false;
    }
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

static inline wabi_error_type
wabi_combiner_operative_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

  res = wabi_vm_alloc(vm, 1);
  if(res) {
    // todo optimize in a single bitwise operation?
    switch(WABI_TAG(v)) {
    case wabi_tag_oper:
    case wabi_tag_bt_oper:
      *res = wabi_val_true;
    default:
      *res = wabi_val_false;
    }
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_combiner_builtin_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

  // todo optimize in a single bitwise operation?
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    switch(WABI_TAG(v)) {
    case wabi_tag_bt_app:
    case wabi_tag_bt_oper:
      *res = wabi_val_true;
    default:
      *res = wabi_val_false;
    }
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_combiner_derived_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

  res = wabi_vm_alloc(vm, 1);
  if(res) {
    // todo optimize in a single bitwise operation?
    switch(WABI_TAG(v)) {
    case wabi_tag_app:
    case wabi_tag_oper:
      *res = wabi_val_true;
    default:
      *res = wabi_val_false;
    }
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_combiner_body_bt(wabi_vm vm, wabi_val f)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->body;
    return wabi_error_none;
  default:
      res = wabi_vm_alloc(vm, 1);
      if(res) {
      *res = wabi_val_nil; // vm->nil; vm->true; vm->false;
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}


static inline wabi_error_type
wabi_combiner_static_env_bt(wabi_vm vm, wabi_val f)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = (wabi_val) WABI_WORD_VAL(((wabi_combiner_derived) f)->static_env);
    return wabi_error_none;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil; // vm->nil;
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}


static inline wabi_error_type
wabi_combiner_parameters_bt(wabi_vm vm, wabi_val f)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->parameters;
    return wabi_error_none;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil; // vm->nil;
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}


static inline wabi_error_type
wabi_combiner_caller_env_name_bt(wabi_vm vm, wabi_val f)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->caller_env_name;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil;
      vm->continuation = (wabi_val) wabi_cont_prev((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}


WABI_BUILTIN_WRAP3(wabi_combiner_builtin_fx, wabi_combiner_fx_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_wrap, wabi_combiner_wrap_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_unwrap, wabi_combiner_unwrap_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_combiner_p, wabi_combiner_combiner_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_applicative_p, wabi_combiner_applicative_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_operative_p, wabi_combiner_operative_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_builtin_p, wabi_combiner_builtin_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_derived_p, wabi_combiner_derived_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_body, wabi_combiner_body_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_static_env, wabi_combiner_static_env_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_parameters, wabi_combiner_parameters_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_caller_env_name, wabi_combiner_caller_env_name_bt)


wabi_error_type
wabi_combiner_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFX(vm, env, "fx", "wabi:fx", wabi_combiner_builtin_fx);
  if(res) return res;

  res = WABI_DEFN(vm, env, "wrap", "wabi:wrap", wabi_combiner_builtin_wrap);
  if(res) return res;
  res = WABI_DEFN(vm, env, "unwrap", "wabi:unwrap", wabi_combiner_builtin_unwrap);
  if(res) return res;
  res = WABI_DEFN(vm, env, "comb?", "wabi:comb?", wabi_combiner_builtin_combiner_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "app?", "wabi:app?", wabi_combiner_builtin_applicative_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "oper?", "wabi:oper?", wabi_combiner_builtin_operative_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/builtin?", "wabi:op/builtin?", wabi_combiner_builtin_builtin_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/derived?", "wabi:op/derived?", wabi_combiner_builtin_derived_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/body", "wabi:op/body", wabi_combiner_builtin_body);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/static-env", "wabi:op/static-env", wabi_combiner_builtin_static_env);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/parameters", "wabi:op/parameters", wabi_combiner_builtin_parameters);
  if(res) return res;
  res = WABI_DEFN(vm, env, "op/caller-env-name", "wabi:op/caller-env-name", wabi_combiner_builtin_caller_env_name);
  return res;
}
