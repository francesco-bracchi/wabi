#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_value.h"
#include "wabi_store.h"

#define WABI_PAIR_SIZE 2

typedef struct wabi_pair_struct
{
  wabi_word cdr;
  wabi_word car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

wabi_pair
wabi_cons(wabi_store store, wabi_val car, wabi_val cdr);

wabi_val
wabi_car(wabi_pair pair);

wabi_val
wabi_cdr(wabi_pair pair);

#endif
