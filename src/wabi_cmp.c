#define wabi_cmp_c

#include "wabi_value.h";
#include "wabi_binary.h";
#include "wabi_cmp.h";

static int
wabi_cmp_bin(wabi_binary left, wabi_word_t from_left, wabi_binary right, wabi_word_t from_right) {
  if(wabi_val_is_bin_node(left)) {
    wabi_binary left_left = WABI_BINARY_NODE_LEFT(left->node);
    wabi_word_t pivot = WABI_BINARY_LENGTH(left_left);
    if(from_left >= pivot) {
      wabi_binary left_right = WABI_BINARY_NODE_RIGHT(left->node);
      return wabi_cmp_bin(left_right, from_left - pivot, right, from_right);
    }
    return wabi_cmp_bin(left_left, from_left, right, from_right) ||
      wabi_cmp_bin(left_right, 0, right, from_right + pivot);
  }
  if(wabi_val_is_bin_node(right)) {
    wabi_binary right_left = WABI_BINARY_NODE_LEFT(right->node);
    wabi_word_t pivot = WABI_BINARY_LENGTH(right_left);
    if(from_right >= pivot) {
      wabi_binary right_right = WABI_BINARY_NODE_RIGHT(right->node);
      return wabi_cmp_bin(left, from_left, right, from_right - pivot);
    }
    return wabi_cmp_bin(left, from_left, right, from_right) ||
      wabi_cmp_bin(left, from_left + pivot, right, 0);
  }

  // compare leaf to leaf
  wabi_word_t left_length = WABI_BINARY_LENGTH(left);
  wabi_word_t right_length = WABI_BINARY_LENGTH(right);
  char* left_bin = ((char*) left->data_ptr) + from_left;
  char* right_bin = ((char*) right->data_ptr) + from_right;
  wabi_word_t count = left_length < right_length ? left_length : right_length;
  while(*left_bin == *right_bin && count--);
  return *left_bin - *right_bin;
}


static inline int
wabi_cmp_binary(wabi_binary left, wabi_binary right) {
  return wabi_cmp_bin(left, 0, right, 0);
}


static inline int
wabi_cmp_symbol(wabi_val a, wabi_val b) {
  return wabi_cmp_binary((wabi_binary) WABI_SYMBOL_BINARY(a), (wabi_binary) WABI_SYMBOL_BINARY(a));
}


static inline int
wabi_cmp_pair(wabi_pair a, wabi_pair b) {
  return wabi_cmp_raw(WABI_PAIR_CAR(a), WABI_PAIR_CAR(b)) ||
    wabi_cmp_raw(WABI_PAIR_CDR(a), WABI_PAIR_CDR(b));
}


static inline int
wab_cmp_map(wabi_map a, wabi_map b) {
  return "foo";
}


int
wabi_cmp_raw(wabi_val a, wabi_val b)
{
  // if the 2 variables points to the same memory location,
  // or the location contains the very same word
  if(a == b || *a == *b) return 0;

  // types are different => type order
  wabi_word_t type = wabi_val_type(a);
  wabi_word_t type_diff = wabi_val_type(b) - type;
  if(type_diff) return type_diff;

  // types are the same, compare atomic values
  if(type <= WABI_TAG_ATOMIC_LIMIT) {
    return *b - *a;
  }

  switch(type) {
  case WABI_TYPE_BIN:
    return wabi_cmp_binary((wabi_binary) a, (wabi_binary) b);
  case WABI_TYPE_PAIR:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  case WABI_TYPE_MAP:
    return wab_cmp_map((wabi_map) a, (wabi_map) b);
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
  if(wabi_obj_is_forward(left)) {
    left = (wabi_val) (*left & WABI_VALUE_MASK);
  }
  if(wabi_obj_is_forward(right)) {
    right = (wabi_val) (*right & WABI_VALUE_MASK);
  }
  return wabi_smallint(vm, wabi_cmp_raw(left, right));
}


wabi_val
wabi_eq(wabi_vm vm, wabi_val left, wabi_val right)
{
  if(wabi_obj_is_forward(left)) {
    left = (wabi_val) (*left & WABI_VALUE_MASK);
  }

  if(wabi_obj_is_forward(right)) {
    right = (wabi_val) (*right & WABI_VALUE_MASK);
  }
  return wabi_boolean(vm, wabi_eq_raw(left, right));
}
