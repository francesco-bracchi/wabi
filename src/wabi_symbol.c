#define wabi_symbol_c

#include <stddef.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_map.h"
#include "wabi_binary.h"
#include "wabi_symbol.h"

#define WABI_SYMBOL_SIZE 1


wabi_symbol
wabi_intern_raw(wabi_vm vm, wabi_binary bin)
{
  wabi_val interned = wabi_map_get_raw((wabi_map) vm->symbol_table, (wabi_val) bin);
  if(interned) {
    return interned;
  }
  wabi_val symbol = (wabi_val) wabi_mem_allocate(vm, 1);
  if(vm->errno) return NULL;
  *symbol = (wabi_word_t) bin | WABI_TAG_SYMBOL;
  vm->symbol_table = (wabi_word_t*) wabi_map_assoc_raw(vm, (wabi_map) vm->symbol_table, (wabi_val) bin, symbol);
  if(vm->errno) return NULL;

  return symbol;
}


wabi_val
wabi_intern(wabi_vm vm, wabi_val bin)
{
  if(! wabi_val_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return (wabi_val) wabi_intern_raw(vm, (wabi_binary) bin);
}
