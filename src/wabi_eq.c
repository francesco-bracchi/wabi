#define wabi_eq_c

#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_hash.h"
#include "wabi_eq.h"


int
wabi_eq_pair(wabi_obj a, wabi_obj b)
{
  return wabi_eq_raw(wabi_car_raw(a), wabi_car_raw(b)) && wabi_eq_raw(wabi_cdr_raw(a), wabi_cdr_raw(b));
}


int
wabi_eq_bin(wabi_obj a, wabi_obj b)
{
  // todo TBD, if the hash fails, it's almost impossible
  // that the 2 things are different with the same hash.
  return 0;
}


int
wabi_eq_raw(wabi_obj a, wabi_obj b)
{
  // same memory reference
  if(a == b) return 1;

  wabi_word_t tag_a = wabi_obj_tag(a);
  wabi_word_t tag_b = wabi_obj_tag(b);

  // follow forwards
  if(tag_a == WABI_TAG_FORWARD) {
    return wabi_eq_raw((wabi_obj) wabi_obj_value(a), b);
  }

  if(tag_b == WABI_TAG_FORWARD) {
    return wabi_eq_raw(a, (wabi_obj) wabi_obj_value(b));
  }

  // types are different
  if(tag_a != tag_b) return 0;

  // symbols
  if(tag_a == WABI_TAG_SYMBOL) {
    return *a == *b
      || wabi_eq_raw((wabi_obj)(*a & WABI_VALUE_MASK),
                     (wabi_obj)(*b & WABI_VALUE_MASK));
  }

  // for types that fits in a word, test that they are the same
  if(tag_a <= WABI_TAG_ATOMIC_LIMIT) {
    return *a == *b;
  }
  // test for hash
  if(wabi_hash_raw(a) != wabi_hash_raw(b)) return 0;

  // test for structural equality
  switch(tag_a) {
  case WABI_TAG_PAIR:
    return wabi_eq_pair(a, b);
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    return wabi_eq_bin(a, b);
    // hatm is missing
  }
  return 1;
}


wabi_obj
wabi_eq(wabi_vm vm, wabi_obj a, wabi_obj b)
{
  return wabi_boolean(vm, wabi_eq_raw(a, b));
}
