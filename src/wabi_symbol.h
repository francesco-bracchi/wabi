#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_value.h"
#include "wabi_vm.h"

#define WABI_SYMBOL_BINARY(s) ((wabi_binary) (*(s) & WABI_VALUE_MASK))

wabi_val
wabi_intern(wabi_vm vm, wabi_val str);

typedef wabi_val wabi_symbol;

#endif