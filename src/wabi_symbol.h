#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_value.h"
#include "wabi_store.h"

typedef wabi_val wabi_symbol;

wabi_symbol
wabi_symbol_new(wabi_store store, wabi_val bin_ref);

#endif
