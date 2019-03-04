#define wabi_types_c

#include "wabi_types.h"
#include "wabi_mem.h"

#include <stdio.h>

wabi_size_t wabi_type_size(wabi_word_t* obj)
{
  wabi_word_t tag = wabi_type_tag(obj);

  if(tag <= WABI_TYPE_TAG_FORWARD)
    return 1;

  if(tag == WABI_TYPE_TAG_PAIR)
    return 2;

  if(tag == WABI_TYPE_TAG_BINARY)
    return (*obj & WABI_TYPE_VALUE_MASK) + 1;

  return 0;
}

wabi_word_t *wabi_smallint_new(long long a)
{
  wabi_word_t *res = wabi_mem_allocate(1);
  *res = (a & WABI_TYPE_VALUE_MASK) | WABI_TYPE_TAG_SMALLINT;
  return res;
}

wabi_word_t* wabi_nil_new()
{
  wabi_word_t *nil = wabi_mem_allocate(1);
  *nil = WABI_VALUE_NIL;
  return nil;
}

wabi_word_t *
wabi_cons(wabi_word_t *car, wabi_word_t *cdr)
{
  wabi_word_t *res = wabi_mem_allocate(2);
  wabi_pair_t *pair = (wabi_pair_t*) res;
  pair->car = (wabi_word_t) car | WABI_TYPE_TAG_PAIR;
  pair->cdr = (wabi_word_t) cdr;

  return res;
}

wabi_word_t *
wabi_car(wabi_word_t *val)
{
  wabi_pair_t *pair = (wabi_pair_t *) val;
  return (wabi_word_t *) (pair->car & WABI_TYPE_VALUE_MASK);
}


wabi_word_t *
wabi_cdr(wabi_word_t *val)
{
  wabi_pair_t *pair = (wabi_pair_t *) val;
  return (wabi_word_t *) (pair->cdr & WABI_TYPE_VALUE_MASK);
}
