#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_err.h"

/**
 * TODO: these can be macros
*/

wabi_val
wabi_car(wabi_val pair)
{
  return (wabi_val)(*pair & WABI_VALUE_MASK);
}

wabi_val
wabi_cdr(wabi_val pair)
{
  return (wabi_val)(*(pair + 1) & WABI_VALUE_MASK);
}

wabi_pair
wabi_cons(wabi_store store, wabi_val car, wabi_val cdr)
{
  wabi_val res = (wabi_val) wabi_store_allocate(store, 2);
    // wabi_store_allocate(store, WABI_PAIR_SIZE);
  if(!res) return NULL;

  *res = WABI_TAG_PAIR | ((wabi_word_t) car);
  *(res + 1) = (wabi_word_t) cdr;
  return (wabi_pair) res;
}
