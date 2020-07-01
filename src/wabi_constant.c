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
wabi_constant_empty_p(const wabi_vm vm) {
  wabi_builtin_predicate(vm, &wabi_is_empty);
}

void
wabi_constant_builtins(const wabi_vm vm, wabi_env env)
{
  wabi_val val;

  val = (wabi_val) wabi_vm_alloc(vm, 6);
  if(vm->ert) return;

  *val = wabi_val_nil;
  wabi_def(vm, env, "nil", val);
  val++;

  *val = wabi_val_true;
  wabi_def(vm, env, "true", val);
  val++;

  *val = wabi_val_false;
  wabi_def(vm, env, "false", val);
  val++;

  *val = wabi_val_ignore;
  wabi_def(vm, env, "ignore", val);
  val++;

  *val = wabi_val_empty;
  wabi_def(vm, env, "emp", val);
  val++;

  *val = wabi_val_zero;
  wabi_def(vm, env, "zero", val);

  wabi_defn(vm, env, "nil?", wabi_constant_nil_p);
  if(vm->ert) return;

  wabi_defn(vm, env, "ignore?", wabi_constant_ignore_p);
  if(vm->ert) return;

  wabi_defn(vm, env, "bool?", wabi_constant_boolean_p);
  if(vm->ert) return;

  wabi_defn(vm, env, "emp?", wabi_constant_empty_p);
  if(vm->ert) return;
}
