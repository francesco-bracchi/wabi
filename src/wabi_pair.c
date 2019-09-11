#define wabi_pair_c

#include <stddef.h>

#include "wabi_pair.h"
#include "wabi_value.h"
#include "wabi_store.h"

/**
 * TODO: these can be macros
*/

wabi_val
wabi_car(wabi_pair pair)
{
  return (wabi_val) pair->car;
}

wabi_val
wabi_cdr(wabi_pair pair)
{
  return (wabi_val) WABI_WORD_VAL(pair->cdr);
}

wabi_pair
wabi_cons(wabi_store store, wabi_val car, wabi_val cdr)
{
  wabi_pair pair = (wabi_pair) wabi_store_heap_alloc(store, WABI_PAIR_SIZE);
  if(!pair) return NULL;
  pair->car = (wabi_word) car;
  pair->cdr = (wabi_word) cdr;
  WABI_SET_TAG(pair, wabi_tag_pair);
  return pair;
}

wabi_val
wabi_nil(wabi_store store)
{
  wabi_val nil;
  nil = (wabi_val) wabi_store_heap_alloc(store, 1);
  *nil = wabi_val_nil;
  return nil;
}
