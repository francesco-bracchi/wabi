#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_map.h"
#include "wabi_collect.h"

#define WABI_SYMBOL_SIZE 1

typedef wabi_word* wabi_symbol;

wabi_symbol
wabi_symbol_new(const wabi_vm vm, const wabi_val bin_ref);


static inline wabi_val
wabi_symbol_to_binary(const wabi_symbol sym)
{
  return (wabi_val) WABI_WORD_VAL(*sym);
}


static inline void
wabi_symbol_copy_val(const wabi_vm vm, const wabi_symbol sym)
{
  wabi_copy_val_size(vm, (wabi_val) sym, WABI_SYMBOL_SIZE);
}


void
wabi_symbol_collect_val(const wabi_vm vm, const wabi_val sym);


void
wabi_symbol_builtins(const wabi_vm vm, const wabi_env env);


static inline int
wabi_is_symbol(const wabi_val v) {
  return WABI_IS(wabi_tag_symbol, v);
}


static inline void
wabi_symbol_hash(const wabi_hash_state state, const wabi_symbol val)
{
  wabi_hash_step(state, "S", 1);
  wabi_hash_val(state, wabi_symbol_to_binary(val));
}

#endif
