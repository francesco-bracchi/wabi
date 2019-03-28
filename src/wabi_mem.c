/**
 * Memory manager
 */

#define wabi_mem_c

#include <string.h>
#include <stdlib.h>

#include "wabi_object.h"
#include "wabi_mem.h"
#include "wabi_vm.h"
#include "wabi_err.h"
#include "wabi_binary.h"

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
}

void
wabi_mem_copy_binary(wabi_vm vm, wabi_obj src)
{
  wabi_size_t len, word_size;

  wabi_binary_leaf_t *new_leaf;
  wabi_obj new_blob;
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


wabi_word_t*
wabi_mem_copy(wabi_vm vm, wabi_word_t *src)
{
  wabi_word_t tag = wabi_obj_tag(src);
  if(tag == WABI_TAG_FORWARD) {
    return (wabi_word_t *) (*src & WABI_VALUE_MASK);
  }
  wabi_word_t* res = vm->mem_alloc;

  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    *res = *src;
    vm->mem_alloc++;
  } else switch(tag) {
    case WABI_TAG_PAIR:
      *res = *src;
      *(res + 1) = *(src + 1);
      vm->mem_alloc += 2;
      break;
    case WABI_TAG_BIN_LEAF:
    case WABI_TAG_BIN_NODE:
      wabi_mem_copy_binary(vm, src);
      break;
    }
  *src = WABI_TAG_FORWARD | ((wabi_word_t) *res);
  return res;
}

void
wabi_mem_collect_step(wabi_vm vm)
{
  wabi_word_t tag, *car, *cdr;

  tag = wabi_obj_tag(vm->mem_scan);

  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    vm->mem_scan++;
  }
  else {
    switch(tag) {
    case WABI_TAG_PAIR:
      car = wabi_mem_copy(vm, (wabi_word_t *) (*vm->mem_scan & WABI_VALUE_MASK));
      cdr = wabi_mem_copy(vm, (wabi_word_t *) *(vm->mem_scan + 1));
      *vm->mem_scan = WABI_TAG_PAIR | (wabi_word_t) car;
      *(vm->mem_scan + 1) = (wabi_word_t) cdr;
      vm->mem_scan += 2;
      break;
    case WABI_TAG_BIN_BLOB:
      vm->mem_scan += (*vm->mem_scan & WABI_VALUE_MASK);
      break;
    case WABI_TAG_BIN_LEAF:
      vm->mem_scan += 3;
      break;
    }
  }
}

void
wabi_mem_collect(wabi_vm vm)
{
  wabi_word_t *wabi_mem_to_space = vm->mem_from_space;
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

  vm->mem_root = wabi_mem_copy(vm, vm->mem_root);

  while(vm->mem_scan < vm->mem_alloc)
    wabi_mem_collect_step(vm);

  free(wabi_mem_to_space);
}

wabi_word_t*
wabi_mem_allocate(wabi_vm vm, wabi_size_t size)
{
  if(vm->mem_alloc + size >= vm->mem_limit)
    wabi_mem_collect(vm);

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
