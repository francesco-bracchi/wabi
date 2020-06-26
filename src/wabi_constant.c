#define wabi_constant_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_builtin.h"



static void
wabi_constant_ignore_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_ignore);
}


static void
wabi_constant_boolean_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_boolean);
}

void
wabi_constant_nil_p(const wabi_vm vm) {
  wabi_builtin_predicate(vm, &wabi_is_nil);
}

void
wabi_constant_builtins(const wabi_vm vm, wabi_env env)
{
  wabi_val val;

  val = (wabi_val) wabi_vm_alloc(vm, 5);
  if(vm->ert) return;

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

  WABI_DEFN(vm, env, "nil?", "nil?", wabi_constant_nil_p);
  if(vm->ert) return;

  WABI_DEFN(vm, env, "ignore?", "ignore?", wabi_constant_ignore_p);
  if(vm->ert) return;

  WABI_DEFN(vm, env, "bool?", "bool?", wabi_constant_boolean_p);
  if(vm->ert) return;
}
