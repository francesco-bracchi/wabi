#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_value.h"
#include "wabi_vm.h"

#define WABI_PAIR_SIZE 2

typedef struct wabi_pair_struct
{
  wabi_word_t cdr;
  wabi_word_t car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

#define WABI_PAIR_CAR(e) ((wabi_val) ((e)->car))
#define WABI_PAIR_CDR(e) ((wabi_val) ((e)->cdr & WABI_VALUE_MASK))

wabi_val
wabi_cons(wabi_vm vm, wabi_val car, wabi_val cdr);

wabi_val
wabi_car(wabi_vm vm, wabi_val pair);

wabi_val
wabi_cdr(wabi_vm vm, wabi_val pair);

wabi_val
wabi_car_raw(wabi_val pair);

wabi_val
wabi_cdr_raw(wabi_val pair);

#endif
