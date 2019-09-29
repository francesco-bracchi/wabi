#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"

typedef wabi_val wabi_symbol;

wabi_symbol
wabi_symbol_new(wabi_vm vm, wabi_val bin_ref);


static inline wabi_val
wabi_symbol_to_binary(wabi_symbol sym)
{
  return WABI_DEREF(sym);
}

#endif
