#define wabi_cmp_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_atomic.h"
#include "wabi_pair.h"
#include "wabi_symbol.h"
#include "wabi_cmp.h"

#include "wabi_pr.h"

static inline int
wabi_cmp_leaves(wabi_binary_leaf left,
                wabi_word_t from_left,
                wabi_word_t len_left,
                wabi_binary_leaf right,
                wabi_word_t from_right,
                wabi_word_t len_right)
{
  wabi_word_t count = (len_left < len_right ? len_left : len_right) - 1;
  char* left_char = WABI_BINARY_DATA_PTR(left) + from_left;
  char* right_char = WABI_BINARY_DATA_PTR(right) + from_right;
  while(*left_char == *right_char && count) {
    count--;
    left_char++;
    right_char++;
  }
  return *left_char == *right_char ? len_left - len_right : *left_char - *right_char;
}


static int
wabi_cmp_bin(wabi_binary left,
             wabi_word_t from_left,
             wabi_word_t len_left,
             wabi_binary right,
             wabi_word_t from_right,
             wabi_word_t len_right)
{
  if(wabi_val_is_bin_node((wabi_val) left)) {
    wabi_binary left_left = WABI_BINARY_NODE_LEFT((wabi_binary_node) left);
    wabi_binary left_right = WABI_BINARY_NODE_RIGHT((wabi_binary_node) left);
    wabi_word_t pivot = WABI_BINARY_LENGTH(left_left);
    if(from_left >= pivot) {
      return wabi_cmp_bin(left_right, from_left - pivot, len_left, right, from_right, len_right);
    }
    wabi_word_t left_len0 = pivot - from_left;
    if(len_left <= left_len0) {
      return wabi_cmp_bin(left_left, from_left, len_left, right, from_right, len_right);
    }
    if(len_right <= left_len0) {
      return wabi_cmp_bin(left_left, from_left, left_len0, right, from_right, len_right);
    }
    int cmp0 = wabi_cmp_bin(left_left, from_left, left_len0, right, from_right, left_len0);
    if(cmp0) return cmp0;
    return wabi_cmp_bin(left_right, 0, len_left - left_len0, right, from_right + left_len0, len_right - left_len0);
  }

  if(wabi_val_is_bin_node((wabi_val) right)) {
    return - wabi_cmp_bin(right, from_right, len_right, left, from_left, len_left);
  }
  return wabi_cmp_leaves((wabi_binary_leaf) left, from_left, len_left,
                         (wabi_binary_leaf) right, from_right, len_right);
}


static inline int
wabi_cmp_binary(wabi_binary left, wabi_binary right) {
  return wabi_cmp_bin(left, 0, WABI_BINARY_LENGTH(left), right, 0, WABI_BINARY_LENGTH(right));
}


static inline int
wabi_cmp_symbol(wabi_symbol a, wabi_symbol b) {
  return wabi_cmp_binary(WABI_SYMBOL_BINARY(a),
                         WABI_SYMBOL_BINARY(b));
}


static inline int
wabi_cmp_pair(wabi_pair a, wabi_pair b) {
  return
    wabi_cmp_raw(WABI_PAIR_CAR(a), WABI_PAIR_CAR(b)) ||
    wabi_cmp_raw(WABI_PAIR_CDR(a), WABI_PAIR_CDR(b));
}


int
wabi_cmp_raw(wabi_val a, wabi_val b)
{
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;

  // types are different => type order
  wabi_word_t type = wabi_val_type(a);
  wabi_word_t type_diff = wabi_val_type(b) - type;
  if(type_diff) {
    return type_diff;
  }
  // types are the same, compare atomic values
  if(type <= WABI_TAG_ATOMIC_LIMIT) {
    return *b - *a;
  }
  switch(type) {
  case WABI_TYPE_BIN:
    return wabi_cmp_binary((wabi_binary) a, (wabi_binary) b);
  case WABI_TYPE_PAIR:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  /* case WABI_TYPE_MAP: */
  /*   return wab_cmp_map((wabi_map) a, (wabi_map) b); */
  case WABI_TYPE_SYMBOL:
    return wabi_cmp_symbol(a, b);
  default:
    return -1;
  }
}


int
wabi_eq_raw(wabi_val left, wabi_val right)
{
  return !wabi_cmp_raw(left, right);
}


wabi_val
wabi_cmp(wabi_vm vm, wabi_val left, wabi_val right)
{
  return wabi_smallint(vm, wabi_cmp_raw(left, right));
}


wabi_val
wabi_eq(wabi_vm vm, wabi_val left, wabi_val right)
{
  return (wabi_val) wabi_boolean(vm, wabi_eq_raw(left, right));
}
