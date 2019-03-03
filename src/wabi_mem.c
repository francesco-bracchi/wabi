/**
 * Memory manager
 */

#define wabi_mem_c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wabi_types.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_err.h"

#define wabi_mem_obj_reference(vm, obj) ((obj) - (vm)->space)

wabi_size_t wabi_mem_size(wabi_vm_t *vm)
{
  return vm->mem_space_limit - vm->mem_space;
}

void wabi_mem_init(wabi_vm_t *vm, wabi_size_t size)
{
  vm->mem_space = (wabi_word_t*) malloc(sizeof(wabi_word_t) * size);
  vm->mem_space_limit = vm->mem_space + size;
  vm->mem_alloc = vm->mem_space;

  vm->mem_from_space = NULL;
  vm->mem_scan = NULL;

  vm->errno = vm->mem_space ? 0 : WABI_ERROR_NOMEM;
}

wabi_word_t *wabi_mem_alloc(wabi_vm_t *vm, wabi_word_t size)
{
  if(vm->mem_alloc + size >= vm->mem_space_limit) {
    wabi_mem_collect(vm);

    if (vm->errno > 0)
      return NULL;

    if(vm->mem_alloc + size >= vm->mem_space_limit) {
      vm->errno = WABI_ERROR_NOMEM;
      return NULL;
    }
  }

  wabi_word_t *res = vm->mem_alloc;
  vm->mem_alloc += size;
  return res;
}

wabi_word_t wabi_mem_copy(wabi_vm_t *vm, wabi_word_t *obj)
{

  if (wabi_is_forward(obj)) {
    return wabi_value(obj);
  } else {

    wabi_word_t *new_obj = vm->mem_alloc;
    wabi_size_t size = wabi_type_size(obj);
    memcpy(new_obj, obj, size * sizeof(wabi_word_t));

    vm->mem_alloc += size;

    wabi_word_t ref = obj - vm->mem_space;
    wabi_word_t forward = wabi_forward(ref);
    *obj = forward;
    return ref;
  }
}

void wabi_mem_collect_step(wabi_vm_t *vm)
{
  wabi_word_t* obj = vm->mem_scan;
  wabi_word_t tag = wabi_tag(obj);

  if(tag <= WABI_TYPE_TAG_FORWARD) {
    vm->mem_scan++;
    return;
  }

  if(tag ==  WABI_TYPE_TAG_PAIR) {
    wabi_pair_t *pair = (wabi_pair_t*) obj;

    wabi_word_t *car = vm->mem_from_space + (pair->car & WABI_TYPE_VALUE_MASK);
    pair->car = wabi_mem_copy(vm, car);

    wabi_word_t *cdr = vm->mem_from_space + (pair->cdr & WABI_TYPE_VALUE_MASK);
    pair->cdr = wabi_mem_copy(vm, cdr);

    vm->mem_scan+=2;
    return;
  }
  vm->errno = WABI_ERROR_TAG_NOT_FOUND;
}

void wabi_mem_collect(wabi_vm_t *vm)
{
  wabi_size_t size = wabi_mem_size(vm);
  vm->mem_from_space = vm->mem_space;
  vm->mem_space = (wabi_word_t*) malloc(sizeof(wabi_word_t) * size);
  vm->mem_space_limit = vm->mem_space + size;

  if (! vm->mem_space) {
    vm->errno = WABI_ERROR_NOMEM;
    return;
  }

  vm->mem_alloc = vm->mem_space;
  vm->mem_scan = vm->mem_space;

  vm->mem_root = vm->mem_space + wabi_mem_copy(vm, vm->mem_root);

  while(vm->mem_scan < vm->mem_alloc) {
    wabi_mem_collect_step(vm);
  }

  free(vm->mem_from_space);
  vm->mem_from_space = NULL;
  vm->mem_scan = NULL;
}

inline wabi_size_t wabi_mem_used(wabi_vm_t *vm)
{
  return vm->mem_alloc - vm->mem_space;
}
