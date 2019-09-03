#define wabi_cmp_c

#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_map.h"
#include "wabi_cmp.h"
#include "wabi_number.h"
#include "wabi_symbol.h"

int
wabi_cmp_leaves(wabi_binary_leaf left,
                wabi_word from_left,
                wabi_word len_left,
                wabi_binary_leaf right,
                wabi_word from_right,
                wabi_word len_right)
{
  wabi_word count = (len_left < len_right ? len_left : len_right) - 1;
  char* left_char = ((char *) left->data_ptr) + from_left;
  char* right_char = ((char *) right->data_ptr) + from_right;
  while(*left_char == *right_char && count) {
    count--;
    left_char++;
    right_char++;
  }
  return *left_char == *right_char ? len_left - len_right : *left_char - *right_char;
}


int
wabi_cmp_bin(wabi_binary left,
             wabi_word from_left,
             wabi_word len_left,
             wabi_binary right,
             wabi_word from_right,
             wabi_word len_right)
{
  if(WABI_TAG((wabi_val) left) == wabi_tag_bin_node) {
    wabi_binary left_left = (wabi_binary) ((wabi_binary_node) left)->left;
    wabi_binary left_right = (wabi_binary) ((wabi_binary_node) left)->right;
    wabi_word pivot = wabi_binary_length(left_left);
    if(from_left >= pivot) {
      return wabi_cmp_bin(left_right, from_left - pivot, len_left, right, from_right, len_right);
    }
    wabi_word left_len0 = pivot - from_left;
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

  if(WABI_TAG((wabi_val) right) == wabi_tag_bin_node) {
    return - wabi_cmp_bin(right, from_right, len_right, left, from_left, len_left);
  }
  return wabi_cmp_leaves((wabi_binary_leaf) left, from_left, len_left,
                         (wabi_binary_leaf) right, from_right, len_right);
}


int
wabi_cmp_binary(wabi_binary left, wabi_binary right)
{
  return wabi_cmp_bin(left, 0, wabi_binary_length(left), right, 0, wabi_binary_length(right));
}

int
wabi_cmp_pair(wabi_pair left, wabi_pair right) {
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}


int
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
      cmp = wabi_cmp(WABI_MAP_ENTRY_KEY(left_entry),
                     WABI_MAP_ENTRY_KEY(right_entry));
      if(cmp) return cmp;
      cmp =  wabi_cmp(WABI_MAP_ENTRY_VALUE(left_entry),
                      WABI_MAP_ENTRY_VALUE(right_entry));
      if(cmp) return cmp;
    }
    wabi_map_iterator_next(&left_iter);
    wabi_map_iterator_next(&right_iter);
  } while(1);
}


/* int */
/* wabi_cmp_env(wabi_env left, wabi_env right) { */
/*   int cmp0; */
/*   do { */
/*     cmp0 = wabi_cmp_map((wabi_map) left->data, */
/*                         (wabi_map) right->data); */
/*     if(cmp0) return cmp0; */

/*     left = (wabi_env) (left->prev & WABI_VALUE_MASK); */
/*     right = (wabi_env) (right->prev & WABI_VALUE_MASK); */
/*     if(left == right) return 0; */
/*     if(left == NULL) return 1; */
/*     if(right == NULL) return -1; */
/*   } while(1); */
/* } */


/* int */
/* wabi_cmp_derived_combiner(wabi_combiner_derived a, wabi_combiner_derived b) */
/* { */
/*   int cmp; */

/*   cmp = wabi_cmp_raw((wabi_val) a->body, (wabi_val) b->body); */
/*   if(cmp) return cmp; */

/*   cmp = wabi_cmp_raw((wabi_val) a->arguments, (wabi_val) b->arguments); */
/*   if(cmp) return cmp; */

/*   cmp = wabi_cmp_raw((wabi_val) a->caller_env_name, (wabi_val) b->caller_env_name); */
/*   if(cmp) return cmp; */

/*   return wabi_cmp_raw((wabi_val) (a->caller_env_name & WABI_VALUE_MASK), */
/*                       (wabi_val) (b->caller_env_name & WABI_VALUE_MASK)); */
/* } */


/* int */
/* wabi_cmp_combiner(wabi_combiner a, wabi_combiner b) */
/* { */
/*   wabi_word tag_a = wabi_val_tag((wabi_val) a); */
/*   wabi_word tag_b = wabi_val_tag((wabi_val) b); */
/*   wabi_word tag_d = tag_b - tag_a; */

/*   if(tag_d) return tag_d >> 56; */

/*   switch(tag_a) { */
/*   case WABI_TAG_OPERATIVE: */
/*   case WABI_TAG_APPLICATIVE: */
/*     return wabi_cmp_derived_combiner((wabi_combiner_derived) a, (wabi_combiner_derived) b); */
/*   case WABI_TAG_BUILTIN_OP: */
/*   case WABI_TAG_BUILTIN_APP: */
/*     return (*((wabi_val) b) & WABI_VALUE_MASK) - (*((wabi_val) a) & WABI_VALUE_MASK); */
/*   } */
/*   return -1; */
/* } */

int
wabi_cmp_fixnum(wabi_fixnum a, wabi_fixnum b) {
  long d = WABI_CAST_INT64(b) - WABI_CAST_INT64(a);
  return d ? (d > 0L ? 1 : -1) : 0;
}

int
wabi_cmp(wabi_val a, wabi_val b)
{
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;
  // types are different => type order
  wabi_word tag = WABI_TAG(a);
  wabi_word tag_diff = tag - WABI_TAG(b);
  if(tag_diff) {
    return (int)(tag_diff >> wabi_word_tag_offset);
  }
  switch(tag) {
  case wabi_tag_constant:
    return (*a - *b);
  case wabi_tag_fixnum:
    return wabi_cmp_fixnum((wabi_fixnum) a, (wabi_fixnum) b);
  case wabi_tag_symbol:
    // todo, since a symbol only refers to binary short circuit here
    return wabi_cmp(WABI_DEREF(a), WABI_DEREF(b));
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    return wabi_cmp_binary((wabi_binary) a, (wabi_binary) b);
  case wabi_tag_pair:
    return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    return wabi_cmp_map((wabi_map) a, (wabi_map) b);
  /* case WABI_TYPE_ENV: */
  /*   return wabi_cmp_env((wabi_env) a, (wabi_env) b); */
  /* case WABI_TYPE_COMBINER: */
  /*   return wabi_cmp_combiner((wabi_combiner) a, (wabi_combiner) b); */
  default:
    return -1;
  }
}

int
wabi_eq(wabi_val left, wabi_val right)
{
  return !wabi_cmp(left, right);
}
