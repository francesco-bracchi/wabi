#define wabi_combiner_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_builtin.h"
#include "wabi_symbol.h"
#include "wabi_atom.h"
#include "wabi_error.h"


wabi_combiner
wabi_operator_builtin_new(const wabi_vm vm,
                          const wabi_binary cname,
                          const wabi_builtin_fun cfun)
{
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  res->c_xtra = (wabi_word) vm->nil;
  WABI_SET_TAG(res, wabi_tag_bt_oper);

  return (wabi_combiner) res;
}


wabi_combiner
wabi_application_builtin_new(const wabi_vm vm,
                             const wabi_binary cname,
                             const wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;
  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  res->c_xtra = (wabi_word) vm->nil;
  WABI_SET_TAG(res, wabi_tag_bt_app);
  return (wabi_combiner) res;
}

//todo: move into wabi_cont.c
wabi_combiner
wabi_combiner_continuation_new(const wabi_vm vm, const wabi_cont cont)
{
  wabi_combiner_continuation res;
  res = (wabi_combiner_continuation) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
  if(vm->ert) return NULL;
  res->cont = (wabi_word) cont;
  WABI_SET_TAG(res, wabi_tag_ct);
  return (wabi_combiner) res;
}


/**
 * Basic vocabulary stuff
 */

static inline void
wabi_combiner_fx(const wabi_vm vm)
{
  wabi_combiner_derived fx;
  wabi_val ctrl, e, fs;
  ctrl = vm->ctrl; // (e fs . ctrl)
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  e = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_symbol(e) && !wabi_atom_is_ignore(vm, e)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fs = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fx = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
  if(vm->ert) return;

  fx->static_env = (wabi_word) ((wabi_cont_call) vm->cont)->env;
  fx->caller_env_name = (wabi_word) e;
  fx->parameters = (wabi_word) fs;
  fx->body = (wabi_word) ctrl;
  fx->compiled_body = (wabi_word) vm->nil;
  WABI_SET_TAG(fx, wabi_tag_oper);
  vm->ctrl = (wabi_val) fx;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont)vm->cont);
}


static void
wabi_combiner_wrap(const wabi_vm vm)
{
  wabi_val ctrl, fx, res;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fx = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  switch(WABI_TAG(fx)) {
  case wabi_tag_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(vm->ert) return;
    wordcopy(res, fx, WABI_COMBINER_DERIVED_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_app);
    break;
  case wabi_tag_bt_oper:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(vm->ert) return;
    wordcopy(res, fx, WABI_COMBINER_BUILTIN_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_bt_app);
    break;

  case wabi_tag_app:
  case wabi_tag_bt_app:
    res = fx;
    break;

  default:
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}



static void
wabi_combiner_unwrap(const wabi_vm vm)
{
  wabi_val ctrl, fn, res;
  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fn = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  switch(WABI_TAG(fn)) {
  case wabi_tag_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    if(vm->ert) return;
    wordcopy(res, fn, WABI_COMBINER_DERIVED_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_oper);
    break;
  case wabi_tag_bt_app:
    res = (wabi_val) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    if(vm->ert) return;
    wordcopy(res, fn, WABI_COMBINER_BUILTIN_SIZE);
    *res = WABI_WORD_VAL(*res);
    WABI_SET_TAG(res, wabi_tag_bt_oper);
    break;

  case wabi_tag_oper:
  case wabi_tag_bt_oper:
    res = fn;
    break;

  default:
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_combiner_combiner_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // todo optimize in a single bitwise operation?
  switch(WABI_TAG(val)) {
  case wabi_tag_app:
  case wabi_tag_oper:
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_combiner_applicative_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // todo optimize in a single bitwise operation?
  switch(WABI_TAG(val)) {
  case wabi_tag_app:
  case wabi_tag_bt_app:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_combiner_operative_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // todo optimize in a single bitwise operation?
  switch(WABI_TAG(val)) {
  case wabi_tag_oper:
  case wabi_tag_bt_oper:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_combiner_builtin_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // todo optimize in a single bitwise operation?
  switch(WABI_TAG(val)) {
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_combiner_derived_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  // todo optimize in a single bitwise operation?
  switch(WABI_TAG(val)) {
  case wabi_tag_app:
  case wabi_tag_oper:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_combiner_cont_p(const wabi_vm vm)
{
  wabi_val ctrl, val, res;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  switch(WABI_TAG(val)) {
  case wabi_tag_ct:
    res = vm->trh;
    break;
  default:
    res = vm->fls;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_combiner_derived_combiner_body(const wabi_vm vm)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (!wabi_combiner_is_derived(val)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = (wabi_val) wabi_combiner_derived_body((wabi_combiner_derived) val);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_combiner_derived_combiner_static_env(const wabi_vm vm)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (!wabi_combiner_is_derived(val)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = (wabi_val) wabi_combiner_derived_static_env((wabi_combiner_derived) val);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_combiner_derived_combiner_parameters(const wabi_vm vm)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (!wabi_combiner_is_derived(val)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = (wabi_val) wabi_combiner_derived_parameters((wabi_combiner_derived) val);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_combiner_derived_combiner_caller_env_name(const wabi_vm vm)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  val = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if (!wabi_combiner_is_derived(val)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = wabi_combiner_derived_caller_env_name((wabi_combiner_derived) val);
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

void
wabi_combiner_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defx(vm, env, "fx", &wabi_combiner_fx);
  if(vm->ert) return;
  wabi_defn(vm, env, "wrap", &wabi_combiner_wrap);
  if(vm->ert) return;
  wabi_defn(vm, env, "unwrap", &wabi_combiner_unwrap);
  if(vm->ert) return;
  wabi_defn(vm, env, "comb?", &wabi_combiner_combiner_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "fn?", &wabi_combiner_applicative_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "fx?", &wabi_combiner_operative_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "cont?", &wabi_combiner_cont_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "builtin?", &wabi_combiner_builtin_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "derived?", &wabi_combiner_derived_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "combiner-body", &wabi_combiner_derived_combiner_body);
  if(vm->ert) return;
  wabi_defn(vm, env, "combiner-static-env", &wabi_combiner_derived_combiner_static_env);
  if(vm->ert) return;
  wabi_defn(vm, env, "combiner-parameters", &wabi_combiner_derived_combiner_parameters);
  if(vm->ert) return;
  /* wabi_defn(vm, env, "combiner-compiled-body", "combiner-compiled-body", wabi_combiner_builtin_compiled_body); */
  /* if(vm->ert) return; */
  /* wabi_defn(vm, env, "combiner-compiled-body-set!", "combiner-compiled-body-set!", wabi_combiner_builtin_compiled_body_set_qmark); */
  /* if(vm->ert) return; */
  wabi_defn(vm, env, "combiner-caller-env-name", wabi_combiner_derived_combiner_caller_env_name);
}
