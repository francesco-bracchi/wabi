#ifndef wabi_cmp_h

#define wabi_cmp_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

int
wabi_cmp(const wabi_val left, const wabi_val right);

int
wabi_eq(const wabi_val left, const wabi_val right);

void
wabi_cmp_builtins(const wabi_vm vm, const wabi_env env);

void
wabi_cmp_lt(const wabi_vm vm);

#endif
