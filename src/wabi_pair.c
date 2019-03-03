#include "wabi_pair.h"

#include "wabi_vm.h"
#include "wabi_mem.h"
#include <string.h>

wabi_word_t *wabi_pair_cons(wabi_vm_t* vm, wabi_word_t *a, wabi_word_t* d)
{
  wabi_pair_t *pair = (wabi_pair_t *) wabi_mem_alloc(vm, 2);
  pair->car = (wabi_word_t*) ((a - vm->mem_space) | WABI_TYPE_TAG_PAIR);
  pair->cdr = (wabi_word_t*) (d - vm->mem_space);
  return (wabi_word_t*) pair;
}

wabi_word_t *wabi_small_new(wabi_vm_t* vm, long long a)
{
  wabi_word_t *num = (wabi_word_t*) wabi_mem_alloc(vm, 1);
  *num = (a & WABI_TYPE_VALUE_MASK) | WABI_TYPE_TAG_SMALLINT;
  return num;
}

wabi_word_t *wabi_const_new(wabi_vm_t* vm, wabi_word_t c)
{
  wabi_word_t *ref = (wabi_word_t*) wabi_mem_alloc(vm, 1);
  *ref = c;
  return ref;
}

wabi_word_t *wabi_binary_new(wabi_vm_t* vm, wabi_word_t size)
{
  // assert(size & WABI_TYPE_TAG_MASK == 0)
  wabi_word_t *res = (wabi_word_t*) wabi_mem_alloc(vm, 1 + size);
  *res = WABI_TYPE_TAG_BINARY | size;
  return res;
}

wabi_word_t *wabi_binary_copy(wabi_vm_t* vm, void* ptr, wabi_word_t size)
{
  wabi_word_t* res = wabi_binary_new(vm, size);
  memcpy(res + 1, ptr, size);
  return res;
}
