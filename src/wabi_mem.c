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

#define WABI_DEBUG 1

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
wabi_mem_free(wabi_vm vm)
{
  free(vm->mem_from_space);
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
  // todo: use wbi_val_type(src)
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
      memcpy(res, src, 2 * WABI_WORD_SIZE);
      vm->mem_alloc+=2;
      break;
    case WABI_TAG_MAP_ARRAY:
    case WABI_TAG_MAP_HASH:
    case WABI_TAG_MAP_ENTRY:
      memcpy(res, src, WABI_MAP_BYTE_SIZE);
      vm->mem_alloc += WABI_MAP_SIZE;
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


inline static void
wabi_mem_collect_pair(wabi_vm vm, wabi_pair pair)
{
  pair->car = (wabi_word_t) wabi_mem_copy_val(vm, (wabi_val) (pair->car | WABI_VALUE_MASK)) | WABI_TAG_PAIR;
  pair->cdr = (wabi_word_t) wabi_mem_copy_val(vm, (wabi_val) pair->cdr);
  vm->mem_scan += 2;
}


inline static void
wabi_mem_collect_symbol(wabi_vm vm, wabi_val sym)
{
  wabi_val new_bin = wabi_mem_copy_val(vm, (wabi_val)(*sym & WABI_VALUE_MASK));
  *sym = (wabi_word_t) new_bin | WABI_TAG_SYMBOL;
  vm->mem_scan++;
}


inline static void
wabi_mem_collect_map_entry(wabi_vm vm,
                           wabi_map_entry map)
{
  wabi_val key = wabi_mem_copy_val(vm, WABI_MAP_ENTRY_KEY(map));
  wabi_val value = wabi_mem_copy_val(vm, WABI_MAP_ENTRY_VALUE(map));
  map->key = (wabi_word_t) key;
  map->value = (wabi_word_t) value | WABI_TAG_MAP_ENTRY;
  vm->mem_scan += 2;
}


inline static void
wabi_mem_collect_map_array(wabi_vm vm,
                           wabi_map_array map)
{
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  if(size) {
    // todo: remove the if or not? the output is the same, number of operations not.
    wabi_map table = WABI_MAP_ARRAY_TABLE(map);
    wabi_map res = (wabi_map) vm->mem_alloc;

    memcpy(res, table, WABI_MAP_BYTE_SIZE * size);
    vm->mem_alloc += size * WABI_MAP_SIZE;
    map->table = (wabi_word_t) res | WABI_TAG_MAP_ARRAY;
  }
  vm->mem_scan += WABI_MAP_SIZE;
}


inline static void
wabi_mem_collect_map_hash(wabi_vm vm,
                          wabi_map_hash map)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_map table = WABI_MAP_HASH_TABLE(map);
  wabi_word_t* res = vm->mem_alloc;

  memcpy(res, table, WABI_MAP_BYTE_SIZE * size);
  vm->mem_alloc += WABI_MAP_SIZE * size;
  map->table = (wabi_word_t) res | WABI_TAG_MAP_HASH;
  vm->mem_scan += WABI_MAP_SIZE;
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
    case WABI_TAG_MAP_ENTRY:
      wabi_mem_collect_map_entry(vm, (wabi_map_entry) vm->mem_scan);
      break;
    case WABI_TAG_MAP_ARRAY:
      wabi_mem_collect_map_array(vm, (wabi_map_array) vm->mem_scan);
      break;
    case WABI_TAG_MAP_HASH:
      wabi_mem_collect_map_hash(vm, (wabi_map_hash) vm->mem_scan);
      break;
    default:
      vm->errno = WABI_ERROR_UNKNOWN;
      break;
    }
  }
}


void
wabi_mem_collect(wabi_vm vm)
{
  vm->mem_to_space = vm->mem_from_space;
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
  vm->errno = WABI_ERROR_NONE;
  free(vm->mem_to_space);
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
  return (wabi_word_t)(vm->mem_limit - vm->mem_from_space);
}
