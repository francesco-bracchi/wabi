#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_mem.h"
#include "wabi_vm.h"
#include "wabi_atomic.h"

wabi_obj
wabi_smallint(wabi_vm vm, int64_t val)
{
  wabi_obj res = wabi_mem_allocate(vm, WABI_SMALLINT_SIZE);
  if(vm->errno) return NULL;
  *res = val | WABI_TAG_SMALLINT;
  return res;
}


wabi_obj
wabi_nil(wabi_vm vm)
{
  wabi_obj res = wabi_mem_allocate(vm, WABI_NIL_SIZE);
  if(vm->errno) return NULL;

  *res = WABI_VALUE_NIL;
  return res;
}


wabi_obj
wabi_boolean(wabi_vm vm, int val)
{
  wabi_obj res = (wabi_obj) wabi_mem_allocate(vm, WABI_BOOLEAN_SIZE);
  if(vm->errno) return NULL;

  *res = val ? WABI_VALUE_TRUE : WABI_VALUE_FALSE;
  return res;
}
