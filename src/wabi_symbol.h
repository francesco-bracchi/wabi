#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_map.h"
#include "wabi_collect.h"

#define WABI_SYMBOL_SIZE 1

typedef wabi_word* wabi_symbol;

wabi_symbol
wabi_symbol_new(wabi_vm vm, wabi_val bin_ref);


static inline wabi_val
wabi_symbol_to_binary(wabi_symbol sym)
{
  return (wabi_val) WABI_WORD_VAL(*sym);
}


static inline void
wabi_symbol_copy_val(wabi_vm vm, wabi_symbol sym)
{
  wabi_copy_val_size(vm, (wabi_val) sym, WABI_SYMBOL_SIZE);
}


void
wabi_symbol_collect_val(wabi_vm vm, wabi_val sym);


wabi_error_type
wabi_symbol_builtins(wabi_vm vm, wabi_env env);

#endif
