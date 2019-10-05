#ifndef wabi_constant_h

#define wabi_constant_h

#include "wabi_vm.h"
#include "wabi_value.h"


void
wabi_constant_builtin_nil(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_true(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_false(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_ignore(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_nil_p(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_ignore_p(wabi_vm vm, wabi_env env);


void
wabi_constant_builtin_boolean_p(wabi_vm vm, wabi_env env);

void
wabi_constant_builtins(wabi_vm vm, wabi_env env);

#endif
