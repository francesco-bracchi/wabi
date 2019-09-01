#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_value.h"
#include "wabi_store.h"

#define WABI_PAIR_SIZE 2

typedef struct wabi_pair_struct
{
  wabi_word_t cdr;
  wabi_word_t car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

#define WABI_CAR(pair) (*((pair) + 1) & WABI_VALUE_MASK)
#define WABI_CDR(pair) (*(pair) & WABI_VALUE_MASK)

wabi_val
wabi_cons(wabi_store store, wabi_val car, wabi_val cdr);

wabi_val
wabi_car(wabi_val pair);

wabi_val
wabi_cdr(wabi_val pair);

#endif
