/**
 * Memory manager
 */

#define wabi_mem_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_mem.h"
#include "wabi_vm.h"
#include "wabi_err.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_hash.h"
#include "wabi_map.h"

#define WABI_MEM_LIMIT (wabi_word_t *)0x00FFFFFFFFFFFFFF


wabi_word_t *
wabi_allocate_space(wabi_word_t size)
{
  return (wabi_word_t *) malloc(WABI_WORD_SIZE * size);
}


void
wabi_mem_init(wabi_vm vm, wabi_size_t size)
{
  vm->mem_size = size;
  vm->mem_from_space = wabi_allocate_space(size);

  if(vm->mem_from_space == NULL) {
    vm->errno = WABI_ERROR_NOMEM;
    return;
  }
  if(vm->mem_from_space + vm->mem_size > WABI_MEM_LIMIT) {
    vm->errno = WABI_ERROR_NOMEM;
    return;
  }

  vm->mem_limit = vm->mem_from_space + vm->mem_size;
  vm->mem_alloc = vm->mem_from_space;
  vm->mem_scan = NULL;
  vm->symbol_table = wabi_map_empty(vm);
}


void
wabi_mem_compact_binary(wabi_vm vm, wabi_val src)
{
  wabi_size_t len, word_size;

  wabi_binary_leaf_t *new_leaf;
  wabi_val new_blob;
  char *data;

  len = *src & WABI_VALUE_MASK;
  word_size = wabi_binary_word_size(len);
  new_leaf = (wabi_binary_leaf_t *) vm->mem_alloc;
  vm->mem_alloc += WABI_BINARY_LEAF_SIZE;

  new_blob = vm->mem_alloc;
  vm->mem_alloc += 1 + word_size;

  data = (char *) (new_blob + 1);

  new_leaf->length = (wabi_word_t) len | WABI_TAG_BIN_LEAF;
  new_leaf->data_ptr = (wabi_word_t) data;

  wabi_binary_compact_raw(src, data);
}


wabi_word_t *
wabi_mem_copy_val(wabi_vm vm, wabi_word_t *src)
{
  wabi_word_t tag = wabi_val_tag(src);
  if(tag == WABI_TAG_FORWARD) {
    return (wabi_word_t *) (*src & WABI_VALUE_MASK);
  }
  wabi_word_t* res = vm->mem_alloc;

  if(tag <= WABI_TAG_ATOMIC_LIMIT || tag == WABI_TAG_SYMBOL) {
    *res = *src;
    vm->mem_alloc++;
  } else switch(tag) {
    case WABI_TAG_PAIR:
    case WABI_TAG_MAP_ARRAY:
    case WABI_TAG_MAP_HASH:
    case WABI_TAG_MAP_ENTRY:
      memcpy(res, src, 2 * WABI_WORD_SIZE);
      vm->mem_alloc+=2;
      break;
    case WABI_TAG_BIN_LEAF:
    case WABI_TAG_BIN_NODE:
      wabi_mem_compact_binary(vm, src);
      break;
    default:
      return NULL;
    }
  *src = WABI_TAG_FORWARD | ((wabi_word_t) res);
  return res;
}


wabi_map
wabi_mem_copy_map(wabi_vm vm, wabi_map map, wabi_word_t size)
{
  wabi_word_t delta = size * WABI_MAP_SIZE;
  wabi_map res = (wabi_map) vm->mem_alloc;
  memcpy(res, map, delta * WABI_WORD_SIZE);
  vm->mem_alloc += delta;
  return res;
}


void
wabi_mem_collect_pair(wabi_vm vm, wabi_pair pair)
{
  pair->car = (wabi_word_t) wabi_mem_copy_val(vm, (wabi_val) (pair->car | WABI_VALUE_MASK)) | WABI_TAG_PAIR;
  pair->cdr = (wabi_word_t) wabi_mem_copy_val(vm, (wabi_val) pair->cdr);
  vm->mem_scan += 2;
}



inline void
wabi_mem_collect_symbol(wabi_vm vm, wabi_val sym)
{
  wabi_val new_bin = wabi_mem_copy_val(vm, (wabi_val)(*sym & WABI_VALUE_MASK));
  *sym = (wabi_word_t) new_bin | WABI_TAG_SYMBOL;
  vm->mem_scan++;
}


void
wabi_mem_collect_step(wabi_vm vm)
{
  wabi_word_t tag;
  tag = wabi_val_tag(vm->mem_scan);
  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    vm->mem_scan++;
  }
  else {
    switch(tag) {
    case WABI_TAG_PAIR:
      wabi_mem_collect_pair(vm, (wabi_pair) vm->mem_scan);
      break;
    case WABI_TAG_BIN_BLOB:
      vm->mem_scan += (*vm->mem_scan & WABI_VALUE_MASK);
      break;
    case WABI_TAG_BIN_LEAF:
      vm->mem_scan += 3;
      break;
    case WABI_TAG_SYMBOL:
      wabi_mem_collect_symbol(vm, vm->mem_scan);
      break;
    }
  }
}


/* inline void */
/* wabi_mem_collect_symbol_table_entry(wabi_vm vm, wabi_hamt_entry entry) */
/* { */
/*   wabi_val key = ENTRY_KEY(entry); */
/*   if(wabi_val_is_forward(key)) { */
/*     wabi_val value = ENTRY_VALUE(entry); */
/*     key = (wabi_val) (*key & WABI_VALUE_MASK); */
/*     value = (wabi_val) (*value & WABI_VALUE_MASK); */
/*     vm->symbol_table = wabi_hamt_assoc(vm, vm->symbol_table, key, value); */
/*   } */
/* } */

/* void */
/* wabi_mem_collect_symbol_table(wabi_vm vm, wabi_hamt_map to_symbol_table) */
/* { */
/*   wabi_hamt_table table = (wabi_hamt_table) (to_symbol_table->table & WABI_VALUE_MASK); */
/*   wabi_size_t size = WABI_POPCNT(to_symbol_table->bitmap); */

/*   for(int j = 0; j < size; j++) { */
/*     wabi_hamt_table row = table + j; */
/*     if(wabi_val_is_hamt_map((wabi_val) row)) { */
/*       wabi_mem_collect_symbol_table(vm, (wabi_hamt_map) row); */
/*     } else { */
/*       wabi_mem_collect_symbol_table_entry(vm, (wabi_hamt_entry) row); */
/*     } */
/*   } */
/* } */


void
wabi_mem_collect(wabi_vm vm)
{
  wabi_word_t *wabi_mem_to_space = vm->mem_from_space;
  // wabi_map to_symbol_table = (wabi_map) vm->symbol_table;
  vm->mem_from_space = wabi_allocate_space(vm->mem_size);
  if(vm->mem_from_space == NULL) {
    vm->errno = WABI_ERROR_NOMEM;
    return;
  }
  if(vm->mem_from_space + vm->mem_size > WABI_MEM_LIMIT) {
    vm->errno = WABI_ERROR_NOMEM;
    return;
  }
  vm->mem_limit = vm->mem_from_space + vm->mem_size;
  vm->mem_alloc = vm->mem_from_space;
  vm->mem_scan = vm->mem_from_space;

  vm->symbol_table = (wabi_word_t*) wabi_map_empty(vm);

  vm->mem_root = wabi_mem_copy_val(vm, vm->mem_root);

  while(vm->mem_scan < vm->mem_alloc) {
    wabi_mem_collect_step(vm);
  }
  // wabi_mem_collect_symbol_table(vm, to_symbol_table);
  free(wabi_mem_to_space);
}


wabi_word_t*
wabi_mem_allocate(wabi_vm vm, wabi_size_t size)
{
  /* if(vm->mem_alloc + size >= vm->mem_limit) */
  /*   wabi_mem_collect(vm); */

  if(vm->mem_alloc + size >= vm->mem_limit) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  wabi_word_t* res = vm->mem_alloc;
  vm->mem_alloc += size;
  return res;
}


wabi_word_t
wabi_mem_used(wabi_vm vm)
{
  // if not running gc!
  return (wabi_word_t)(vm->mem_alloc - vm->mem_from_space);
}


wabi_word_t
wabi_mem_total(wabi_vm vm)
{
  // if not running gc!
  return (wabi_word_t)(vm->mem_alloc - vm->mem_from_space);
}
