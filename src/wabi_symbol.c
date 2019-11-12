#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"

wabi_symbol
wabi_symbol_new(wabi_vm vm,
                wabi_val binref)
{
  wabi_symbol res;
  wabi_val new_table;
  res = (wabi_symbol) wabi_map_get((wabi_map) vm->symbol_table, binref);
  if(res) return res;

  if(wabi_vm_has_rooms(vm, WABI_SYMBOL_SIZE)) {
    res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
    res->binref = (wabi_word) binref;
    res->uid = wabi_hash(binref);
    WABI_SET_TAG(res, wabi_tag_symbol);
    new_table = (wabi_val) wabi_map_assoc(vm, (wabi_map) vm->symbol_table, binref, (wabi_val) res);
    if(new_table) {
      vm->symbol_table = new_table;
      return res;
    }
  }
  return NULL;
}
