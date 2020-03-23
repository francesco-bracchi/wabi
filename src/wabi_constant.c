#define wabi_constant_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


static inline wabi_error_type
wabi_constant_nil_p(wabi_vm vm,
                    wabi_val v)
{
  wabi_val res;
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = *v == wabi_val_nil ? wabi_val_true : wabi_val_false;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_constant_ignore_p(wabi_vm vm,
                       wabi_val v)
{
  wabi_val res;
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = *v == wabi_val_ignore ? wabi_val_true : wabi_val_false;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}


static inline wabi_error_type
wabi_constant_boolean_p(wabi_vm vm,
                        wabi_val v)
{
  wabi_val res;
  res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = *v == (wabi_val_true || *v == wabi_val_false) ? wabi_val_true : wabi_val_false;
    vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
    vm->control = res;
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_constant_builtin_nil_p, wabi_constant_nil_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_ignore_p, wabi_constant_ignore_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_boolean_p, wabi_constant_boolean_p)


wabi_error_type
wabi_constant_builtins(wabi_vm vm, wabi_env env)
{
  wabi_val val;
  wabi_error_type res;

  val = (wabi_val) wabi_vm_alloc(vm, 5);
  if(val) {
    *val = wabi_val_nil;
    WABI_DEF(vm, env, "nil", val);
    val++;

    *val = wabi_val_true;
    WABI_DEF(vm, env, "true", val);
    val++;

    *val = wabi_val_false;
    WABI_DEF(vm, env, "false", val);
    val++;

    *val = wabi_val_ignore;
    WABI_DEF(vm, env, "ignore", val);
    val++;

    *val = wabi_val_zero;
    WABI_DEF(vm, env, "zero", val);
  }

  res = WABI_DEFN(vm, env, "nil?", "nil?", wabi_constant_builtin_nil_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "ignore?", "ignore?", wabi_constant_builtin_ignore_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "bool?", "bool?", wabi_constant_builtin_boolean_p);
  return res;
}
