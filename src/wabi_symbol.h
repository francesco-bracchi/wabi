#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_map.h"

#define WABI_SYMBOL_SIZE 1

typedef wabi_word* wabi_symbol;

static inline wabi_symbol
wabi_symbol_new(const wabi_vm vm,
                const wabi_val binref)
{
  wabi_symbol res;
  wabi_map tbl;

  res = (wabi_symbol) wabi_map_get((wabi_map) vm->stbl, binref);
  if(res) return res;

  res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
  if(vm->ert) return NULL;

  *res = (wabi_word) binref;
  WABI_SET_TAG(res, wabi_tag_symbol);

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, res);
  if(vm->ert) return NULL;
  vm->stbl = (wabi_val) tbl;

  return res;
}

static inline wabi_val
wabi_symbol_to_binary(const wabi_symbol sym)
{
  return (wabi_val) WABI_WORD_VAL(*sym);
}


void
wabi_symbol_collect_val(const wabi_vm vm,
                        const wabi_val sym);


void
wabi_symbol_builtins(const wabi_vm vm,
                     const wabi_env env);


static inline int
wabi_is_symbol(const wabi_val v) {
  return WABI_IS(wabi_tag_symbol, v);
}


#endif
