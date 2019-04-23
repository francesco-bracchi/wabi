#ifndef wabi_eq_h

#define wabi_eq_h

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"

wabi_val
wabi_eq(wabi_vm vm, wabi_val a, wabi_val b);

int
wabi_eq_raw(wabi_val a, wabi_val b);

#endif
