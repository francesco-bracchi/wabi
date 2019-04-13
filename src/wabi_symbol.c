#define wabi_symbol_c

#include <stddef.h>
#include <stdio.h>

#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_hamt.h"

#define WABI_SYMBOL_SIZE 1

wabi_obj
wabi_symbol(wabi_vm vm, wabi_obj bin)
{
  if(! wabi_obj_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_obj res = wabi_mem_allocate(vm, WABI_SYMBOL_SIZE);
  if(vm->errno) return NULL;
  *res =  ((wabi_word_t) bin) | WABI_TAG_SYMBOL;
  return res;
}


wabi_obj
wabi_intern(wabi_vm vm, wabi_obj bin)
{
  if(! wabi_obj_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_obj interned = wabi_hamt_get_raw(vm->symbol_table, bin);
  if(interned) {
    return interned;
  }
  wabi_obj symbol = (wabi_obj) wabi_mem_allocate(vm, 1);
  if(vm->errno) return NULL;
  *symbol = (wabi_word_t) bin | WABI_TAG_SYMBOL;
  vm->symbol_table = (wabi_word_t*) wabi_hamt_assoc(vm, vm->symbol_table, bin, symbol);
  if(vm->errno) return NULL;

  return symbol;
}
