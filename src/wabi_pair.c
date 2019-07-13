#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_err.h"

wabi_val
wabi_car_raw(wabi_val pair)
{
  return (wabi_val)(*pair & WABI_VALUE_MASK);
}

wabi_val
wabi_cdr_raw(wabi_val pair)
{
  return (wabi_val)(*(pair + 1) & WABI_VALUE_MASK);
}

wabi_val
wabi_cons(wabi_vm vm, wabi_val car, wabi_val cdr)
{
  wabi_val res = (wabi_val) wabi_vm_allocate(vm, 2);
    // wabi_vm_allocate(vm, WABI_PAIR_SIZE);
  if(vm->errno) return NULL;

  *res = WABI_TAG_PAIR | ((wabi_word_t) car);
  *(res + 1) = (wabi_word_t) cdr;
  return res;
}

wabi_val
wabi_car(wabi_vm vm, wabi_val pair)
{
  if(!wabi_val_is_pair(pair)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return wabi_car_raw(pair);
}

wabi_val
wabi_cdr(wabi_vm vm, wabi_val pair)
{
  if(!wabi_val_is_pair(pair)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return wabi_cdr_raw(pair);
}
