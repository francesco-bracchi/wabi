#define wabi_cmp_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_atomic.h"
#include "wabi_pair.h"
#include "wabi_symbol.h"
#include "wabi_combiner.h"
#include "wabi_map.h"
#include "wabi_cmp.h"
#include "wabi_env.h"
#include "wabi_vm.h"

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
      // is this ever visited?
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
wabi_cmp_symbol(wabi_symbol left, wabi_symbol right) {
  return wabi_cmp_binary(WABI_SYMBOL_BINARY(left),
                         WABI_SYMBOL_BINARY(right));
}


static inline int
wabi_cmp_pair(wabi_pair left, wabi_pair right) {
  int cmp0 = wabi_cmp_raw(WABI_PAIR_CAR(left), WABI_PAIR_CAR(right));
  if(cmp0) return cmp0;
  return wabi_cmp_raw(WABI_PAIR_CDR(left), WABI_PAIR_CDR(right));
}

static inline int
wabi_cmp_map(wabi_map left, wabi_map right)
{
  wabi_map_iter_t left_iter, right_iter;
  wabi_map_entry left_entry, right_entry;
  int cmp;
  wabi_map_iterator_init(&left_iter, left);
  wabi_map_iterator_init(&right_iter, right);

  do {
    left_entry = wabi_map_iterator_current(&left_iter);
    right_entry = wabi_map_iterator_current(&right_iter);

    if(!left_entry && !right_entry) {
      return 0;
    }
    else if(!right_entry) {
      return 1;
    }
    else if(!left_entry) {
      return -1;
    }
    else {
      cmp = wabi_cmp_raw(WABI_MAP_ENTRY_KEY(left_entry),
                         WABI_MAP_ENTRY_KEY(right_entry));
      if(cmp) return cmp;
      cmp =  wabi_cmp_raw(WABI_MAP_ENTRY_VALUE(left_entry),
                          WABI_MAP_ENTRY_VALUE(right_entry));
      if(cmp) return cmp;
    }
    wabi_map_iterator_next(&left_iter);
    wabi_map_iterator_next(&right_iter);
  } while(1);
}


static inline int
wabi_cmp_env(wabi_env left, wabi_env right) {
  int cmp0;
  do {
    cmp0 = wabi_cmp_map((wabi_map) left->data,
                        (wabi_map) right->data);
    if(cmp0) return cmp0;

    left = (wabi_env) (left->prev & WABI_VALUE_MASK);
    right = (wabi_env) (right->prev & WABI_VALUE_MASK);
    if(left == right) return 0;
    if(left == NULL) return 1;
    if(right == NULL) return -1;
  } while(1);
}


static inline int
wabi_cmp_derived_combiner(wabi_combiner_derived a, wabi_combiner_derived b)
{
  int cmp;

  cmp = wabi_cmp_raw((wabi_val) a->body, (wabi_val) b->body);
  if(cmp) return cmp;

  cmp = wabi_cmp_raw((wabi_val) a->arguments, (wabi_val) b->arguments);
  if(cmp) return cmp;

  cmp = wabi_cmp_raw((wabi_val) a->caller_env_name, (wabi_val) b->caller_env_name);
  if(cmp) return cmp;

  return wabi_cmp_raw((wabi_val) (a->caller_env_name & WABI_VALUE_MASK),
                      (wabi_val) (b->caller_env_name & WABI_VALUE_MASK));
}


static inline int
wabi_cmp_combiner(wabi_combiner a, wabi_combiner b)
{
  wabi_word_t tag_a = wabi_val_tag((wabi_val) a);
  wabi_word_t tag_b = wabi_val_tag((wabi_val) b);
  wabi_word_t tag_d = tag_b - tag_a;

  if(tag_d) return tag_d >> 56;

  switch(tag_a) {
  case WABI_TAG_OPERATIVE:
  case WABI_TAG_APPLICATIVE:
    return wabi_cmp_derived_combiner((wabi_combiner_derived) a, (wabi_combiner_derived) b);
  case WABI_TAG_BUILTIN_OP:
  case WABI_TAG_BUILTIN_APP:
    return (*((wabi_val) b) & WABI_VALUE_MASK) - (*((wabi_val) a) & WABI_VALUE_MASK);
  }
  return -1;
}

int
wabi_cmp_raw(wabi_val a, wabi_val b)
{
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;

  // types are different => type order
  wabi_word_t type = wabi_val_type(a);
  wabi_word_t type_diff = type - wabi_val_type(b);
  if(type_diff) {
    return (int)(type_diff >> 56);
  }
  // types are the same, compare atomic values
  if(type <= WABI_TAG_ATOMIC_LIMIT) {
    return *a - *b;
  }
  switch(type) {
  case WABI_TYPE_BIN:
    return wabi_cmp_binary((wabi_binary) a, (wabi_binary) b);
  case WABI_TYPE_PAIR:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  case WABI_TYPE_MAP:
    return wabi_cmp_map((wabi_map) a, (wabi_map) b);
  case WABI_TYPE_SYMBOL:
    return wabi_cmp_symbol(a, b);
  case WABI_TYPE_ENV:
    return wabi_cmp_env((wabi_env) a, (wabi_env) b);
  case WABI_TYPE_COMBINER:
    return wabi_cmp_combiner((wabi_combiner) a, (wabi_combiner) b);
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
