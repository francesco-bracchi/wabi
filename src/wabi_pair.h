#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_object.h"
#include "wabi_vm.h"

#define WABI_PAIR_SIZE 2

typedef struct wabi_pair_struct
{
  wabi_word_t car;
  wabi_word_t cdr;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;

wabi_obj
wabi_cons(wabi_vm vm, wabi_obj car, wabi_obj cdr);

wabi_obj
wabi_car(wabi_vm vm, wabi_obj pair);

wabi_obj
wabi_cdr(wabi_vm vm, wabi_obj pair);

wabi_obj
wabi_car_raw(wabi_obj pair);

wabi_obj
wabi_cdr_raw(wabi_obj pair);

#endif
