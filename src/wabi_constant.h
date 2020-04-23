#ifndef wabi_constant_h

#define wabi_constant_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_error.h"


wabi_error_type
wabi_constant_builtins(wabi_vm vm, wabi_env env);


static inline int
wabi_is_nil(wabi_val v) {
  return *v == wabi_val_nil;
}

#endif
