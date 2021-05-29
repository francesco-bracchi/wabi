#define wabi_cmp_c

#include <stddef.h>
#include "wabi_value.h"
#include "wabi_binary.h"
#include "wabi_list.h"
#include "wabi_map.h"
#include "wabi_number.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_error.h"
#include "wabi_builtin.h"
#include "wabi_place.h"
#include "wabi_vector.h"
#include "wabi_atom.h"
#include "wabi_meta.h"
#include "wabi_cmp.h"
#include "wabi_builtin.h"

static inline int
wabi_cmp_fixnum(wabi_fixnum a, wabi_fixnum b) {
  long d = WABI_CAST_INT64(b) - WABI_CAST_INT64(a);
  return d ? (d > 0L ? 1 : -1) : 0;
}

static inline int
wabi_cmp_bin_leaves(const wabi_binary_leaf left,
                       wabi_word from_left,
                       wabi_word len_left,
                       const wabi_binary_leaf right,
                       wabi_word from_right,
                       wabi_word len_right)
{
  wabi_word count;
  char *left_char, *right_char;

  count = (len_left < len_right ? len_left : len_right) - 1;
  left_char = ((char *) left->data_ptr) + from_left;
  right_char = ((char *) right->data_ptr) + from_right;
  while(*left_char == *right_char && (count > 0)) {
    count--;
    left_char++;
    right_char++;
  }
  // return *left_char == *right_char ? len_left - len_right : *left_char - *right_char;
  if (*left_char > *right_char) return 1;
  if (*left_char < *right_char) return -1;

  if (len_left > len_right) return 1;
  if (len_left < len_right) return -1;

  return 0;
}

static inline int
wabi_cmp_bin_bin(const wabi_binary left,
                    wabi_word from_left,
                    wabi_word len_left,
                    const wabi_binary right,
                    wabi_word from_right,
                    wabi_word len_right)
{
  wabi_binary left_left, left_right;
  wabi_word pivot, left_len0;
  int cmp;
  if(!len_left && !len_right ) {
    return 0;
  }
  if(!len_left ) {
    return 1;
  }
  if (!len_right) {
    return -1;
  }
  if(WABI_IS(wabi_tag_bin_node, left)) {
    left_left = (wabi_binary) ((wabi_binary_node) left)->left;
    left_right = (wabi_binary) ((wabi_binary_node) left)->right;
    pivot = wabi_binary_length(left_left);

    if(from_left >= pivot) {
      return wabi_cmp_bin_bin(left_right, from_left - pivot, len_left, right, from_right, len_right);
    }
    left_len0 = pivot - from_left;
    if(len_left <= left_len0) {
      // is this ever visited?
      return wabi_cmp_bin_bin(left_left, from_left, len_left, right, from_right, len_right);
    }
    if(len_right <= left_len0) {
      return wabi_cmp_bin_bin(left_left, from_left, left_len0, right, from_right, len_right);
    }
    cmp = wabi_cmp_bin_bin(left_left, from_left, left_len0, right, from_right, left_len0);
    if(cmp) return cmp;
    return wabi_cmp_bin_bin(left_right, 0, len_left - left_len0, right, from_right + left_len0, len_right - left_len0);
  }

  if(WABI_IS(wabi_tag_bin_node, right)) {
    return - wabi_cmp_bin_bin(right, from_right, len_right, left, from_left, len_left);
  }
  return wabi_cmp_bin_leaves((wabi_binary_leaf) left, from_left, len_left,
                                (wabi_binary_leaf) right, from_right, len_right);
}

static inline int
wabi_cmp_bin(const wabi_binary left, const wabi_binary right)
{
  return wabi_cmp_bin_bin(left, 0, wabi_binary_length(left), right, 0, wabi_binary_length(right));
}

static inline int
wabi_cmp_map(const wabi_map left,
             const wabi_map right)
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

static inline int
wabi_cmp_pair(const wabi_pair left, const wabi_pair right)
{
  int cmp0 = wabi_cmp(wabi_car(left), wabi_car(right));
  if(cmp0) return cmp0;
  return wabi_cmp(wabi_cdr(left), wabi_cdr(right));
}

static inline int
wabi_cmp_vector(const wabi_vector left,
                const wabi_vector right)
{
  wabi_vector_iter_t left_iter, right_iter;
  wabi_val l, r;
  int cmp;

  cmp = wabi_vector_size(right) - wabi_vector_size(left);
  if(cmp) return cmp;

  wabi_vector_iter_init(&left_iter, left);
  wabi_vector_iter_init(&right_iter, right);

  for(;;) {
    l = wabi_vector_iter_current(&left_iter);
    r = wabi_vector_iter_current(&right_iter);
    if(!l && !r) {
      return 0;
    }
    if(!r) {
      return 1;
    }
    if(!l) {
      return -1;
    }
    cmp = wabi_cmp(l, r);
    if(cmp) return cmp;
    wabi_vector_iter_next(&left_iter);
    wabi_vector_iter_next(&right_iter);
  }
}

static inline int
wabi_cmp_env(wabi_env left,
             wabi_env right)
{
  if(left->uid == right->uid) return 0;
  if(left->uid > right->uid) return 1;
  return -1;
}

static inline int
wabi_cmp_bt(const wabi_combiner_builtin a, const wabi_combiner_builtin b)
{
  return wabi_cmp_fixnum((wabi_fixnum) a, (wabi_fixnum) b);
}

static inline int
wabi_cmp_derived(const wabi_combiner_derived a, const wabi_combiner_derived b)
{
    int cmp;
    cmp =  wabi_cmp(wabi_combiner_derived_static_env(a),
                    wabi_combiner_derived_static_env(b));
    if(cmp) return cmp;

    cmp = wabi_cmp(wabi_combiner_derived_body(a),
                   wabi_combiner_derived_body(b));
    if(cmp) return cmp;

    cmp = wabi_cmp(wabi_combiner_derived_parameters(a),
                   wabi_combiner_derived_parameters(a));
    if(cmp) return cmp;

    return wabi_cmp(wabi_combiner_derived_caller_env_name(a),
                    wabi_combiner_derived_caller_env_name(b));

}

static inline int
wabi_cmp_cont(wabi_cont a, wabi_cont b)
{
  // TODO consider the case of wabi_cont_done
  wabi_word tag;
  wabi_word diff;
  int cmp;
  while(a != wabi_cont_done && b != wabi_cont_done) {
    tag = WABI_TAG(a);
    diff = tag - WABI_TAG(b);
    if(diff < 0) return -1;
    if(diff > 0) return 1;

    switch (tag) {
    case wabi_tag_cont_eval:
      break;

    case wabi_tag_cont_apply:
      cmp = wabi_cmp((wabi_val)((wabi_cont_apply)a)->args,
                     (wabi_val)((wabi_cont_apply)b)->args);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_apply)a)->env,
                     (wabi_val)((wabi_cont_apply)b)->env);
      if (cmp)
        return cmp;
      break;

    case wabi_tag_cont_call:
      cmp = wabi_cmp((wabi_val)((wabi_cont_call)a)->combiner,
                     (wabi_val)((wabi_cont_call)b)->combiner);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_call)a)->env,
                     (wabi_val)((wabi_cont_call)b)->env);
      if (cmp)
        return cmp;
      break;

    case wabi_tag_cont_sel:
      cmp = wabi_cmp((wabi_val)((wabi_cont_sel)a)->left,
                     (wabi_val)((wabi_cont_sel)b)->left);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_sel)a)->right,
                     (wabi_val)((wabi_cont_sel)b)->right);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_sel)a)->env,
                     (wabi_val)((wabi_cont_sel)b)->env);
      if (cmp)
        return cmp;
      break;

    case wabi_tag_cont_args:
      cmp = wabi_cmp((wabi_val)((wabi_cont_args)a)->data,
                     (wabi_val)((wabi_cont_args)b)->data);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_args)a)->done,
                     (wabi_val)((wabi_cont_args)b)->done);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_args)a)->env,
                     (wabi_val)((wabi_cont_args)b)->env);
      if (cmp)
        return cmp;
      break;

    case wabi_tag_cont_def:
      cmp = wabi_cmp((wabi_val)((wabi_cont_def)a)->pattern,
                     (wabi_val)((wabi_cont_def)b)->pattern);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_def)a)->env,
                     (wabi_val)((wabi_cont_def)b)->env);
      if (cmp)
        return cmp;
      break;

    case wabi_tag_cont_prog:
      cmp = wabi_cmp((wabi_val)((wabi_cont_prog)a)->expressions,
                     (wabi_val)((wabi_cont_prog)b)->expressions);
      if (cmp)
        return cmp;
      cmp = wabi_cmp((wabi_val)((wabi_cont_prog)a)->env,
                     (wabi_val)((wabi_cont_prog)b)->env);
      if (cmp)
        return cmp;
      break;
    default:
      printf("IMPOSSIBLE!\n");
      return 0xFFFFFF;
    }
    a = wabi_cont_pop(a);
    b = wabi_cont_pop(b);
  }

  if (a == wabi_cont_done) {
    if (b == wabi_cont_done) {
      return 0;
    }
    return -1;
  }
  return 1;
}

static inline int
wabi_cmp_meta(wabi_meta a,
              wabi_meta b)
{
  int cmp;
  while (a != wabi_meta_empty && b != wabi_meta_empty) {
    cmp = wabi_cmp(wabi_meta_tag(a), wabi_meta_tag(b));
    if(cmp) return cmp;
    cmp = wabi_cmp_cont(wabi_meta_cont(a), wabi_meta_cont(b));
    if(cmp) return cmp;
    a = wabi_meta_pop(a);
    b = wabi_meta_pop(b);
  }
  if (a == wabi_meta_empty) {
    if (b == wabi_meta_empty) {
      return 0;
    }
    return -1;
  }
  return 1;
}

static inline int wabi_cmp_ct(wabi_combiner_continuation a,
                              wabi_combiner_continuation b)
{
  int cmp;
  cmp = wabi_cmp_cont((wabi_cont) wabi_combiner_continuation_cont(a),
                 (wabi_cont) wabi_combiner_continuation_cont(b));
  if(cmp) return cmp;
  cmp = wabi_cmp_meta(wabi_combiner_continuation_atem(a),
                      wabi_combiner_continuation_atem(b));
  if(cmp) return cmp;

}

static inline int
wabi_cmp_place(const wabi_place left,
               const wabi_place right)
{
  if(left->uid == right->uid) return 0;
  if(left->uid > right->uid) return 1;
  return -1;
}

int
wabi_cmp(const wabi_val a, const wabi_val b)
{
  wabi_word tag;
  wabi_word diff;
  // if the 2 values are the very same, they are equal :|
  if(a == b) return 0;
  // types are different => type order
  tag = WABI_TAG(a);

  diff = tag - WABI_TAG(b);
  switch(tag) {
  case wabi_tag_fixnum:
    if(wabi_is_fixnum(b)) {
      return wabi_cmp_fixnum((wabi_fixnum) a, (wabi_fixnum) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_symbol:
    if (wabi_is_symbol(b)) {
      return wabi_cmp(wabi_symbol_to_binary((wabi_symbol)a),
                      wabi_symbol_to_binary((wabi_symbol)b));
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_atom:
    if (wabi_is_atom(b)) {
      return wabi_cmp(wabi_atom_to_binary((wabi_symbol)a),
                      wabi_atom_to_binary((wabi_symbol)b));
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    if(wabi_is_binary(b)) {
      return wabi_cmp_bin((wabi_binary) a, (wabi_binary) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_pair:
    if(wabi_is_pair(b)) {
      return wabi_cmp_pair((wabi_pair) a, (wabi_pair) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    if (wabi_is_map(b)) {
      return wabi_cmp_map((wabi_map)a, (wabi_map)b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_vector_digit:
  case wabi_tag_vector_deep:
    if(wabi_is_vector(b)) {
      return wabi_cmp_vector((wabi_vector) a, (wabi_vector) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_env:
    if(wabi_is_env(b)) {
      return wabi_cmp_env((wabi_env) a, (wabi_env) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_app:
  case wabi_tag_oper:
    if(WABI_TAG(b) == tag) {
      return wabi_cmp_derived((wabi_combiner_derived) a, (wabi_combiner_derived) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    if(WABI_TAG(b) == tag) {
      return wabi_cmp_bt((wabi_combiner_builtin) a, (wabi_combiner_builtin) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_place:
    if(wabi_is_place(b)) {
      return wabi_cmp_place((wabi_place) a, (wabi_place) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  case wabi_tag_ct:
    if (WABI_TAG(b) == tag) {
      return wabi_cmp_ct((wabi_combiner_continuation) a, (wabi_combiner_continuation) b);
    }
    return (int)(diff >> wabi_word_tag_offset);
  default:
    return -10000;
  }
}

int
wabi_eq(const wabi_val left, const wabi_val right)
{
  wabi_word tag;
  wabi_word diff;

  // if the 2 values are the very same, they are equal :|
  if(left == right) return 1;
  // types are different => they are not equal
  tag = WABI_TAG(left);
  diff = tag - WABI_TAG(right);
  if(diff) return 0;

  switch(tag) {
  case wabi_tag_fixnum:
    return (*left == *right);
  default:
    return !wabi_cmp(left, right);
  }
}
