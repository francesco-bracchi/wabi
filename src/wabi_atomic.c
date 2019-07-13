#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"
#include "wabi_atomic.h"


wabi_val
wabi_smallint(wabi_vm vm, int64_t val)
{
  wabi_val res = wabi_vm_allocate(vm, WABI_SMALLINT_SIZE);
  if(vm->errno) return NULL;
  *res = val | WABI_TAG_SMALLINT;
  return res;
}


wabi_val
wabi_nil(wabi_vm vm)
{
  wabi_val res = wabi_vm_allocate(vm, WABI_NIL_SIZE);
  if(vm->errno) return NULL;

  *res = WABI_VALUE_NIL;
  return res;
}


wabi_val
wabi_boolean(wabi_vm vm, int val)
{
  wabi_val res = (wabi_val) wabi_vm_allocate(vm, WABI_BOOLEAN_SIZE);
  if(vm->errno) return NULL;

  *res = val ? WABI_VALUE_TRUE : WABI_VALUE_FALSE;
  return res;
}
