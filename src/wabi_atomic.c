#define wabi_atomic_c

#include <stdint.h>
#include "wabi_atomic.h"
#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_mem.h"


void
wabi_smallint(int64_t val, wabi_obj* res, int* errno)
{
  wabi_mem_allocate(WABI_SMALLINT_SIZE, res, errno);
  if(*errno != WABI_ERROR_NONE) return;
  **res = val | WABI_TAG_SMALLINT;
}


void
wabi_nil(wabi_obj *res, int* errno)
{
  wabi_mem_allocate(WABI_NIL_SIZE, res, errno);
  if(*errno != WABI_ERROR_NONE) return;

  **res = WABI_VALUE_NIL;
}


void
wabi_boolean(int val, wabi_obj *res, wabi_error *errno)
{
  wabi_mem_allocate(WABI_BOOLEAN_SIZE, res, errno);
  if(*errno != WABI_ERROR_NONE) return;

  **res = val ? WABI_VALUE_TRUE : WABI_VALUE_FALSE;
}
