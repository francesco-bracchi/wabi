#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_number.h"

wabi_fixnum
wabi_fixnum_new(wabi_store store,
                int64_t val)
{
  wabi_val res = wabi_store_heap_alloc(store, 1);
  if(! res) return NULL;
  *res = val;
  WABI_SET_TAG(res, wabi_tag_fixnum);
  return res;
}
