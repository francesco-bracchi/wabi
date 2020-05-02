#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"

static inline void
wabi_symbol_set_stbl(wabi_vm vm, wabi_val binref, wabi_val sym) {
  wabi_map tbl;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, sym);
  if(tbl) {
    vm->stbl = (wabi_val) tbl;
  }
}

wabi_symbol
wabi_symbol_new(wabi_vm vm,
                wabi_val binref)
{
  wabi_symbol res;
  wabi_val new_table;
  if(! binref) return NULL;
  res = (wabi_symbol) wabi_map_get((wabi_map) vm->stbl, binref);
  if(res) return res;

  res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
  if(res) {
    *res = (wabi_word) binref;
    WABI_SET_TAG(res, wabi_tag_symbol);
    wabi_symbol_set_stbl(vm, binref, res);
  }
  return res;
}


void
wabi_symbol_collect_val(wabi_vm vm, wabi_val sym)
{
  wabi_val binref, sym0;
  binref = wabi_symbol_to_binary(sym);
  binref = wabi_copy_val(vm, binref);
  *sym = (wabi_word) binref;
  WABI_SET_TAG(sym, wabi_tag_symbol);
  vm->stor.scan+= WABI_SYMBOL_SIZE;

  if(wabi_map_get((wabi_map) vm->stbl, binref))
    return;

  // wabi_symbol_set_stbl(vm, binref, sym);
}
