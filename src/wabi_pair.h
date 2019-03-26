#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_object.h"

#define WABI_PAIR_SIZE 2

void
wabi_cons(wabi_obj car, wabi_obj cdr, wabi_obj* res, int *errno);

void
wabi_car(wabi_obj val, wabi_obj* res, int *errno);

void
wabi_cdr(wabi_obj val, wabi_obj* res, int *errno);

inline wabi_obj
wabi_car_raw(wabi_obj pair)
{
  return (wabi_obj)(*pair & WABI_VALUE_MASK);
}

inline wabi_obj
wabi_cdr_raw(wabi_obj pair)
{
  return (wabi_obj)(*(pair + 1) & WABI_VALUE_MASK);
}

#endif
