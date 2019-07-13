#define wabi_symbol_c

#include <stddef.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_store.h"
#include "wabi_vm.h"
#include "wabi_map.h"
#include "wabi_binary.h"
#include "wabi_symbol.h"

#define WABI_SYMBOL_SIZE 1


wabi_symbol
wabi_intern_raw(wabi_store store, wabi_binary bin)
{
  wabi_val interned = wabi_map_get_raw((wabi_map) store->symbol_table, (wabi_val) bin);
  if(interned) {
    return interned;
  }
  wabi_val symbol = (wabi_val) wabi_store_allocate(store, 1);
  if(!symbol) return NULL;
  *symbol = (wabi_word_t) bin | WABI_TAG_SYMBOL;
  wabi_map table = wabi_map_assoc_raw(store, (wabi_map) store->symbol_table, (wabi_val) bin, symbol);
  if(! table) {
    return NULL;
  }
  store->symbol_table = (wabi_word_t *) table;
  return symbol;
}


wabi_val
wabi_intern(wabi_vm vm, wabi_val bin)
{
  if(! wabi_val_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_val res = (wabi_val) wabi_intern_raw(&(vm->store), (wabi_binary) bin);
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}
