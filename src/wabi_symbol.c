#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"

wabi_symbol
wabi_symbol_new(wabi_vm vm,
                wabi_val bin_ref)
{
  wabi_symbol res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = (wabi_word) bin_ref;
    WABI_SET_TAG(res, wabi_tag_symbol);
    return (wabi_symbol) res;
  }
  return NULL;
}

wabi_val
wabi_symbol_to_binary(wabi_symbol sym)
{
  return WABI_DEREF(sym);
}
