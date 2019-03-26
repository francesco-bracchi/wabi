#define wabi_pair_c

#include "wabi_pair.h"
#include "wabi_object.h"
#include "wabi_mem.h"
#include "wabi_err.h"


void
wabi_cons(wabi_obj car, wabi_obj cdr, wabi_obj *res, int *errno)
{
  wabi_mem_allocate(WABI_PAIR_SIZE, res, errno);
  if(*errno != WABI_ERROR_NONE) return;

  **res = WABI_TAG_PAIR | ((wabi_word_t) car);
  *(*res + 1) = (wabi_word_t) cdr;
}

void
wabi_car(wabi_obj pair, wabi_obj *res, int *errno)
{
  if(!wabi_obj_is_pair(pair)) {
    *errno = WABI_ERROR_TYPE_MISMATCH;
    return;
  }
  *res = wabi_car_raw(pair);
}

void
wabi_cdr(wabi_obj pair, wabi_obj *res, int *errno)
{
  if(!wabi_obj_is_pair(pair)) {
    *errno = WABI_ERROR_TYPE_MISMATCH;
    return;
  }
  *res = wabi_cdr_raw(pair);
}
