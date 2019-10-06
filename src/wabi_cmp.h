#ifndef wabi_cmp_h

#define wabi_cmp_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

int
wabi_cmp(wabi_val left, wabi_val right);

void
wabi_cmp_builtins(wabi_vm vm, wabi_env env);

#endif
