#define wabi_eq_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_hash.h"
#include "wabi_eq.h"

int
wabi_eq_pair(wabi_val a, wabi_val b)
{
  return
    wabi_eq_raw(wabi_car_raw(a), wabi_car_raw(b)) &&
    wabi_eq_raw(wabi_cdr_raw(a), wabi_cdr_raw(b));
}

int
wabi_eq_binary(wabi_binary a, wabi_binary b)
{
  wabi_size_t length_a = wabi_binary_length_raw(a);
  wabi_size_t length_b = wabi_binary_length_raw(b);
  return (length_a == length_b) && wabi_eq_binary_sub(a, b, 0, length_a);
}

int
wabi_eq_binary_sub(wabi_binary a,
                   wabi_binary b,
                   wabi_size_t from,
                   wabi_size_t len)
{
  switch(wabi_val_tag(a)) {
  case WABI_TAG_BIN_LEAF:
    switch(wabi_val_tag(b)) {
    case WABI_TAG_BIN_LEAF:
      return wabi_eq_binary_leaf_leaf(a, b, from, len);
    case WABI_TAG_BIN_NODE:
      return wabi_eq_binary_leaf_node(a, b, from, len);
    }
  case WABI_TAG_BIN_NODE:
    switch(wabi_val_tag(b)) {
    case WABI_TAG_BIN_LEAF:
      return wabi_eq_binary_node_leaf(a, b, from, len);
    case WABI_TAG_BIN_NODE:
      return wabi_eq_binary_node_node(a, b, from, len);
    }
  }
}


int
wabi_eq_binary_leaf_leaf(wabi_binary_leaf a,
                         wabi_binary_leaf b,
                         wabi_size_t from,
                         wabi_size_t len)
{
  return
  wabi_size_t size = MIN(a->length, b->length);
  wabI_size_t max =
}

int
wabi_eq_map(wabi_val a, wabi_val b)
{
}

int
wabi_eq_raw(wabi_val a, wabi_val b)
{
  // same memory reference
  if(a == b) return 1;

  wabi_word_t tag_a = wabi_val_tag(a);
  wabi_word_t tag_b = wabi_val_tag(b);

  // follow forwards
  if(tag_a == WABI_TAG_FORWARD) {
    return wabi_eq_raw((wabi_val) wabi_val_value(a), b);
  }

  if(tag_b == WABI_TAG_FORWARD) {
    return wabi_eq_raw(a, (wabi_val) wabi_val_value(b));
  }

  // types are different
  if(tag_a != tag_b) return 0;

  // symbols
  if(tag_a == WABI_TAG_SYMBOL) {
    return *a == *b;
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


wabi_val
wabi_eq(wabi_vm vm, wabi_val a, wabi_val b)
{
  return wabi_boolean(vm, wabi_eq_raw(a, b));
}
