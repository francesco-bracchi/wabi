/**
 * Memory manager
 */

#define wabi_mem_c

#include <stdlib.h>
#include <string.h>

#include "wabi_types.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_err.h"


wabi_size_t wabi_mem_size(wabi_vm_t *vm)
{
  return vm->mem_space_limit - vm->mem_space;
}

void wabi_mem_init(wabi_vm_t *vm, wabi_size_t size)
{
  vm->mem_space = (wabi_word_t*) malloc(sizeof(wabi_word_t) * size);
  vm->mem_space_limit = vm->mem_space + size;
  vm->mem_alloc = vm->mem_space;

  vm->mem_to_space = NULL;
  vm->mem_scan = NULL;

  vm->errno = vm->mem_space ? WABI_ERROR_NOMEM : 0;
}

wabi_word_t* wabi_mem_malloc(wabi_vm_t *vm, wabi_word_t size)
{
  if(vm->mem_alloc + size >= vm->mem_space_limit) {
    wabi_mem_collect(vm, vm->mem_space);

    if (vm->errno > 0)
      return NULL;

    if(vm->mem_alloc + size >= vm->mem_space_limit) {
      vm->errno = WABI_ERROR_NOMEM;
      return NULL;
    }
  }

  wabi_word_t* res = vm->mem_alloc;
  vm->mem_alloc += size;
  return res;
}

/* void wabi_mem_collect(wabi_vm_t* vm, wabi_word* root) */
/* { */
/*   vm->mem_to_space = vm->mem_from_space; */
/*   vm->mem_from_space = (wabi_word_t*) malloc(sizeof(wabi_word_t) * vm->size); */
/*   vm->mem_alloc = vm->mem_from_space; */
/*   vm->mem_scan = vm->mem_alloc; */
/*   *root = &wabi_mem_copy(vm, root); */
/*   while(vm->mem_scan < vm->mem_alloc) */
/*     wabi_mem_collect_step(vm); */
/* } */


/* void wabi_mem_collect_step(wabi_vm_t* vm) */
/* { */
/*   wabi_word* obj = vm->mem_scan; */
/*   vm->mem_scan++; */
/*   *obj = wabi_mem_copy(vm, obj); */
/* } */

/* wabi_word* wabi_mem_copy(wabi_vm_t* vm, wabi_word* obj) */
/* { */
/*   if(wabi_is_forward(obj)) { */
/*     return wabi_value(obj); */
/*   } else { */
/*     wabi_size_t size = wabi_size(obj); */
/*     wabi_word* new_obj = vm->mem_alloc; */
/*     wabi_word ref = new_obj - vm->from_space; */
/*     memcpy(new_obj, obj, size); */
/*     vm->mem_alloc += size; */
/*     *obj = ref || WABI_TAG_FORWARD; */
/*     return ref; */
/*   } */
/* } */
