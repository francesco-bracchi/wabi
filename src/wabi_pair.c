#define wabi_pair_c

#include "wabi_pair.h"
#include "wabi_object.h"
#include "wabi_mem.h"
#include "wabi_err.h"

void
wabi_cons(wabi_obj car, wabi_obj cdr, wabi_obj* res, int *errno)
{
  wabi_mem_allocate(WABI_PAIR_SIZE, res, errno);
  wabi_obj pair = *res;
  if(*errno != WABI_ERROR_NONE) return;

  *pair = WABI_TAG_PAIR | ((wabi_word_t) car);
  *(pair + 1) = (wabi_word_t) cdr;
}

void
wabi_car(wabi_obj pair, wabi_obj* res, int *errno)
{
  *res = (wabi_word_t *) (*pair & WABI_VALUE_MASK);
}

void
wabi_cdr(wabi_obj pair, wabi_obj* res, int *errno)
{
  *res = (wabi_word_t *) (*(pair + 1) & WABI_VALUE_MASK);
}
