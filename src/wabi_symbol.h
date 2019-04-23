#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_value.h"
#include "wabi_vm.h"

#define WABI_SYMBOL_BINARY(s) (s & WABI_VALUE_MASK)

wabi_val
wabi_intern(wabi_vm vm, wabi_val str);

#endif
