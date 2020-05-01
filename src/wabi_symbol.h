#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_store.h"

#define WABI_SYMBOL_SIZE 1

typedef wabi_word* wabi_symbol;

wabi_symbol
wabi_symbol_new(wabi_vm vm, wabi_val bin_ref);


static inline wabi_val
wabi_symbol_to_binary(wabi_symbol sym)
{
  return (wabi_val) WABI_WORD_VAL(*sym);
}


static void
wabi_symbol_copy_val(wabi_store store, wabi_symbol sym)
{
  wabi_store_copy_val_size(store, (wabi_val) sym, WABI_SYMBOL_SIZE);
}
#endif
