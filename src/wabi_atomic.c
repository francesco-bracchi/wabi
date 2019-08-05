#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_store.h"
#include "wabi_vm.h"
#include "wabi_atomic.h"


wabi_val
wabi_smallint_raw(wabi_store store, int64_t val)
{
  wabi_val res = wabi_store_allocate(store, WABI_SMALLINT_SIZE);
  if(! res) return NULL;
  *res = val | WABI_TAG_SMALLINT;
  return res;
}


wabi_val
wabi_nil_raw(wabi_store store)
{
  wabi_val res = wabi_store_allocate(store, WABI_NIL_SIZE);
  if(! res) return NULL;

  *res = WABI_VALUE_NIL;
  return res;
}


wabi_val
wabi_boolean_raw(wabi_store store, int val)
{
  wabi_val res = (wabi_val) wabi_store_allocate(store, WABI_BOOLEAN_SIZE);
  if(! res) return NULL;

  *res = val ? WABI_VALUE_TRUE : WABI_VALUE_FALSE;
  return res;
}

wabi_val
wabi_ignore_raw(wabi_store store) {
  wabi_val res = (wabi_val) wabi_store_allocate(store, WABI_IGNORE_SIZE);
  if(! res) return NULL;
  *res = WABI_VALUE_IGNORE;
  return res;
}

wabi_val
wabi_smallint(wabi_vm vm, int64_t val)
{
  wabi_val res = wabi_smallint_raw(&(vm->store), val);
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}


wabi_val
wabi_nil(wabi_vm vm)
{
  wabi_val res = wabi_nil_raw(&(vm->store));
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}


wabi_val
wabi_boolean(wabi_vm vm, int val)
{
  wabi_val res = wabi_boolean_raw(&(vm->store), val);
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}


wabi_val
wabi_ignore(wabi_vm vm) {
  wabi_val res = wabi_ignore_raw(&(vm->store));
  if(! res) {
    vm->errno = WABI_ERROR_NOMEM;
    return NULL;
  }
  return res;
}
