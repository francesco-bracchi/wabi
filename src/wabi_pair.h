#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_value.h"
#include "wabi_vm.h"

#define WABI_PAIR_SIZE 2

typedef struct wabi_pair_struct
{
  wabi_word cdr;
  wabi_word car;
} wabi_pair_t;

typedef wabi_pair_t* wabi_pair;


/** CY775VF */
/** CY775FV */

static inline wabi_pair
wabi_cons(wabi_vm vm, wabi_val car, wabi_val cdr)
{
  wabi_pair pair = (wabi_pair) wabi_vm_alloc(vm, WABI_PAIR_SIZE);
  pair->car = (wabi_word) car;
  pair->cdr = (wabi_word) cdr;
  WABI_SET_TAG(pair, wabi_tag_pair);
  return pair;
}


static inline wabi_val
wabi_car(wabi_pair pair)
{
  return (wabi_val) pair->car;
}


static inline wabi_val
wabi_cdr(wabi_pair pair)
{
  return (wabi_val) WABI_WORD_VAL(pair->cdr);
}

#endif
