#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_err.h"
#include "wabi_mem.h"

wabi_obj
wabi_car_raw(wabi_obj pair)
{
  return (wabi_obj)(*pair & WABI_VALUE_MASK);
}

wabi_obj
wabi_cdr_raw(wabi_obj pair)
{
  return (wabi_obj)(*(pair + 1) & WABI_VALUE_MASK);
}

wabi_obj
wabi_cons(wabi_vm vm, wabi_obj car, wabi_obj cdr)
{
  wabi_obj res = (wabi_obj) wabi_mem_allocate(vm, 2);
    // wabi_mem_allocate(vm, WABI_PAIR_SIZE);
  if(vm->errno) return NULL;

  *res = WABI_TAG_PAIR | ((wabi_word_t) car);
  *(res + 1) = (wabi_word_t) cdr;
  return res;
}

wabi_obj
wabi_car(wabi_vm vm, wabi_obj pair)
{
  if(!wabi_obj_is_pair(pair)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return wabi_car_raw(pair);
}

wabi_obj
wabi_cdr(wabi_vm vm, wabi_obj pair)
{
  if(!wabi_obj_is_pair(pair)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return wabi_cdr_raw(pair);
}
