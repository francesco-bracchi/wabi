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
  if(res) {
    res->c_ptr = (wabi_word) cfun;
    res->c_name = (wabi_word) cname;
    WABI_SET_TAG(res, wabi_tag_bt_oper);
  }
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
  if(res) {
    res->c_ptr = (wabi_word) cfun;
    res->c_name = (wabi_word) cname;
    WABI_SET_TAG(res, wabi_tag_bt_app);
  }
  return (wabi_combiner) res;
}

wabi_combiner
wabi_combiner_continuation_new(wabi_vm vm, wabi_cont cont)
{
  wabi_combiner_continuation res;
  res = (wabi_combiner_continuation) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
  if(res) {
    res->cont = (wabi_word) cont;
    WABI_SET_TAG(res, wabi_tag_ct_app);
  }
  return (wabi_combiner) res;
}



void
wabi_combiner_collect_val(wabi_store store, wabi_combiner c)
{
  wabi_word tag;

  wabi_combiner_continuation cc;

  tag = WABI_TAG(c);
  switch(tag) {
  case wabi_tag_app:
  case wabi_tag_oper:
    ((wabi_combiner_derived) c)->static_env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(((wabi_combiner_derived) c)->static_env));
    ((wabi_combiner_derived) c)->caller_env_name =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_combiner_derived) c)->caller_env_name);
    ((wabi_combiner_derived) c)->parameters =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_combiner_derived) c)->parameters);
    ((wabi_combiner_derived) c)->body =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_combiner_derived) c)->body);
    ((wabi_combiner_derived) c)->compiled_body =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_combiner_derived) c)->compiled_body);
    WABI_SET_TAG(c, tag);
    store->scan += WABI_COMBINER_DERIVED_SIZE;
    return;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    ((wabi_combiner_builtin) c)->c_name =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_combiner_builtin) c)->c_name);
    if(((wabi_combiner_builtin) c)->c_xtra) {
      ((wabi_combiner_builtin) c)->c_xtra =
        (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(((wabi_combiner_builtin) c)->c_ptr));
    }
    store->scan += WABI_COMBINER_BUILTIN_SIZE;
    return;

  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    ((wabi_combiner_continuation) c)->cont =
      (wabi_word) wabi_store_copy_val(store, (wabi_val) wabi_combiner_continuation_cont((wabi_combiner_continuation) c));
    WABI_SET_TAG(c, tag);
    store->scan += WABI_COMBINER_CONTINUATION_SIZE;
    return;
  }
}


/**
 * Basic vocabulary stuff
 */

static inline wabi_error_type
wabi_combiner_builtin_fx(wabi_vm vm)
{
  wabi_combiner_derived fx;
  wabi_val ctrl, e, fs;
  ctrl = vm->control; // (e (a b) (+ a b))
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    e = wabi_car((wabi_pair) ctrl);
    if(!WABI_IS(wabi_tag_symbol, e) && *e != wabi_val_ignore) {
      return wabi_error_type_mismatch;
    }
    ctrl = wabi_cdr((wabi_pair) ctrl); // ((a b) (+ a b) (pr "ok"))
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      fs = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl); // ((+ a b) (pr "ok"))
      if(WABI_IS(wabi_tag_pair, ctrl)) {
        fx = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
        if(fx) {
          fx->static_env = (wabi_word) ((wabi_cont_call) vm->continuation)->env;
          fx->caller_env_name = (wabi_word) e;
          fx->parameters = (wabi_word) fs;
          fx->body = (wabi_word) ctrl;
          fx->compiled_body = (wabi_word) vm->nil;
          WABI_SET_TAG(fx, wabi_tag_oper);

          vm->control = (wabi_val) fx;
          vm->continuation = (wabi_val) wabi_cont_next((wabi_cont)vm->continuation);
          return wabi_error_none;
        }
        return wabi_error_nomem;
      }
    }
  }
  return wabi_error_bindings;
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
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  case wabi_tag_ct_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
    if(! res) return wabi_error_nomem;

    memcpy(res, fx, sizeof(wabi_combiner_continuation_t));
    WABI_SET_TAG(res, wabi_tag_ct_app);
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;

  case wabi_tag_app:
  case wabi_tag_bt_app:
  case wabi_tag_ct_app:
    vm->control = fx;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
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
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  case wabi_tag_ct_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
    if(! res) return wabi_error_nomem;

    memcpy(res, fn, sizeof(wabi_combiner_continuation_t));
    WABI_SET_TAG(res, wabi_tag_ct_oper);
    vm->control = res;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    return wabi_error_none;
  case wabi_tag_oper:
  case wabi_tag_bt_oper:
  case wabi_tag_ct_oper:
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = fn;
    return wabi_error_none;
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
    case wabi_tag_oper:
    case wabi_tag_bt_app:
    case wabi_tag_bt_oper:
    case wabi_tag_ct_app:
    case wabi_tag_ct_oper:
      *res = wabi_val_true;
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    case wabi_tag_ct_app:
      *res = wabi_val_true;
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    case wabi_tag_ct_oper:
      *res = wabi_val_true;
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

static inline wabi_error_type
wabi_combiner_cont_p_bt(wabi_vm vm, wabi_val v)
{
  wabi_val res;

  res = wabi_vm_alloc(vm, 1);
  if(res) {
    // todo optimize in a single bitwise operation?
    switch(WABI_TAG(v)) {
    case wabi_tag_ct_app:
    case wabi_tag_ct_oper:
      *res = wabi_val_true;
      break;
    default:
      *res = wabi_val_false;
      break;
    }
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->body;
    return wabi_error_none;
  default:
      res = wabi_vm_alloc(vm, 1);
      if(res) {
      *res = wabi_val_nil; // vm->nil; vm->true; vm->false;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}


static inline wabi_error_type
wabi_combiner_compiled_body_bt(wabi_vm vm, wabi_val f)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->compiled_body;
    return wabi_error_none;
  default:
      res = wabi_vm_alloc(vm, 1);
      if(res) {
      *res = wabi_val_nil; // vm->nil; vm->true; vm->false;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}

static inline wabi_error_type
wabi_combiner_compiled_body_set_qmark_bt(wabi_vm vm, wabi_val f, wabi_val cf)
{
  wabi_val res;
  switch(WABI_TAG(f)) {
  case wabi_tag_oper:
  case wabi_tag_app:
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    ((wabi_combiner_derived) f)->compiled_body = (wabi_word) cf;
    vm->control = cf;
    return wabi_error_none;
  default:
      res = wabi_vm_alloc(vm, 1);
      if(res) {
      *res = wabi_val_nil; // vm->nil; vm->true; vm->false;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = (wabi_val) WABI_WORD_VAL(((wabi_combiner_derived) f)->static_env);
    return wabi_error_none;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil; // vm->nil;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->parameters;
    return wabi_error_none;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil; // vm->nil;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
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
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = (wabi_val) ((wabi_combiner_derived) f)->caller_env_name;
    return wabi_error_none;
  default:
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_val_nil;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      vm->control = res;
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
}

WABI_BUILTIN_WRAP1(wabi_combiner_builtin_wrap, wabi_combiner_wrap_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_unwrap, wabi_combiner_unwrap_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_combiner_p, wabi_combiner_combiner_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_applicative_p, wabi_combiner_applicative_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_operative_p, wabi_combiner_operative_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_builtin_p, wabi_combiner_builtin_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_cont_p, wabi_combiner_cont_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_derived_p, wabi_combiner_derived_p_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_body, wabi_combiner_body_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_static_env, wabi_combiner_static_env_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_parameters, wabi_combiner_parameters_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_caller_env_name, wabi_combiner_caller_env_name_bt)
WABI_BUILTIN_WRAP1(wabi_combiner_builtin_compiled_body, wabi_combiner_compiled_body_bt)
WABI_BUILTIN_WRAP2(wabi_combiner_builtin_compiled_body_set_qmark, wabi_combiner_compiled_body_set_qmark_bt)


wabi_error_type
wabi_combiner_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFX(vm, env, "fx", "fx", wabi_combiner_builtin_fx);
  if(res) return res;
  res = WABI_DEFN(vm, env, "wrap", "wrap", wabi_combiner_builtin_wrap);
  if(res) return res;
  res = WABI_DEFN(vm, env, "unwrap", "unwrap", wabi_combiner_builtin_unwrap);
  if(res) return res;
  res = WABI_DEFN(vm, env, "comb?", "comb?", wabi_combiner_builtin_combiner_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "app?", "app?", wabi_combiner_builtin_applicative_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "oper?", "oper?", wabi_combiner_builtin_operative_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "cont?", "cont?", wabi_combiner_builtin_cont_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "builtin?", "builtin?", wabi_combiner_builtin_builtin_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "derived?", "derived?", wabi_combiner_builtin_derived_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-body", "combiner-body", wabi_combiner_builtin_body);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-static-env", "combiner-static-env", wabi_combiner_builtin_static_env);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-parameters", "combiner-parameters", wabi_combiner_builtin_parameters);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-compiled-body", "combiner-compiled-body", wabi_combiner_builtin_compiled_body);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-compiled-body-set!", "combiner-compiled-body-set!", wabi_combiner_builtin_compiled_body_set_qmark);
  if(res) return res;
  res = WABI_DEFN(vm, env, "combiner-caller-env-name", "combiner-caller-env-name", wabi_combiner_builtin_caller_env_name);
  return res;
}
