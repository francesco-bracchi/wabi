#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

wabi_symbol
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
