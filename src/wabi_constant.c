#define wabi_constant_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


static inline void
wabi_constant_nil_p(wabi_vm vm,
                    wabi_env env,
                    wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == wabi_val_nil ? wabi_val_true : wabi_val_false;
    vm->control = res;
    return;
  }
  vm->errno = wabi_error_nomem;
}


static inline void
wabi_constant_ignore_p(wabi_vm vm,
                       wabi_env env,
                       wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == wabi_val_ignore ? wabi_val_true : wabi_val_false;
    vm->control = res;
    return;
  }
  vm->errno = wabi_error_nomem;
}


static inline void
wabi_constant_boolean_p(wabi_vm vm,
                        wabi_env env,
                        wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == (wabi_val_true || *v == wabi_val_false) ? wabi_val_true : wabi_val_false;
    vm->control = res;
    return;
  }
  vm->errno = wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_constant_builtin_nil_p, wabi_constant_nil_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_ignore_p, wabi_constant_ignore_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_boolean_p, wabi_constant_boolean_p)


void
wabi_constant_builtins(wabi_vm vm, wabi_env env)
{
  wabi_val val;
  val = (wabi_val) wabi_vm_alloc(vm, 5);

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

  WABI_DEFN(vm, env, "nil?", "wabi:nil?", wabi_constant_builtin_nil_p);
  WABI_DEFN(vm, env, "ignore?", "wabi:nil?", wabi_constant_builtin_ignore_p);
  WABI_DEFN(vm, env, "bool?", "wabi:bool?", wabi_constant_builtin_boolean_p);
}
