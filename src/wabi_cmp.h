#ifndef wabi_cmp_h

#define wabi_cmp_h

#include "wabi_value.h"
#include "wabi_vm.h"

int
wabi_cmp_raw(wabi_val left, wabi_val right);

int
wabi_eq_raw(wabi_val left, wabi_val right);

wabi_val
wabi_eq(wabi_vm vm, wabi_val left, wabi_val right);

wabi_val
wabi_cmp(wabi_vm vm, wabi_val left, wabi_val right);

#endif
