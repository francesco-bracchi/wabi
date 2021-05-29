#ifndef wabi_value_h

#define wabi_value_h

#include "wabi_word.h"

typedef int wabi_error_code;

typedef enum wabi_tag_enum {
  wabi_tag_fixnum         = 0x0000000000000000,
  wabi_tag_atom           = 0x0100000000000000,
  wabi_tag_symbol         = 0x0200000000000000,
  wabi_tag_forward        = 0x0300000000000000,
  wabi_tag_pair           = 0x0400000000000000,

  wabi_tag_bin_blob       = 0x0500000000000000,
  wabi_tag_bin_leaf       = 0x0600000000000000,
  wabi_tag_bin_node       = 0x0700000000000000,

  wabi_tag_map_array      = 0x0800000000000000,
  wabi_tag_map_entry      = 0x0900000000000000,
  wabi_tag_map_hash       = 0x0A00000000000000,

  wabi_tag_app            = 0x0B00000000000000, // 0101 1
  wabi_tag_bt_app         = 0x0C00000000000000, // 0110 0
  wabi_tag_oper           = 0x0D00000000000000, // 0110 1
  wabi_tag_bt_oper        = 0x0E00000000000000, // 0111 0

  wabi_tag_env            = 0x0F00000000000000,

  wabi_tag_place          = 0x1000000000000000,
  wabi_tag_alien          = 0x1100000000000000,
  wabi_tag_tagged         = 0x1200000000000000,

  wabi_tag_cont_eval      = 0x1300000000000000,
  wabi_tag_cont_apply     = 0x1400000000000000,
  wabi_tag_cont_call      = 0x1500000000000000,
  wabi_tag_cont_sel       = 0x1600000000000000,
  wabi_tag_cont_args      = 0x1700000000000000,
  wabi_tag_cont_def       = 0x1800000000000000,
  wabi_tag_cont_prog      = 0x1900000000000000,

  wabi_tag_ct             = 0x1A00000000000000,

  wabi_tag_vector_digit   = 0x1B00000000000000,
  wabi_tag_vector_deep    = 0x1C00000000000000,
  wabi_tag_meta           = 0x1D00000000000000
} wabi_tag;

typedef wabi_word* wabi_val;

typedef wabi_val wabi_expr;

#define WABI_WORD_TAG(word) ((word) & wabi_word_tag_mask)

#define WABI_WORD_VAL(word) ((word) & wabi_word_value_mask)

#define WABI_TAG(val) WABI_WORD_TAG(*((wabi_val) val))

#define WABI_DEREF(val) ((wabi_val) WABI_WORD_VAL(*val))

#define WABI_SET_TAG(val,tag) (*((wabi_word *) val) |= tag)

#define WABI_IS(tag, val) (WABI_TAG((wabi_word *) val) == tag)

static const wabi_word wabi_val_zero   = wabi_tag_fixnum;

#define WABI_IS_VAL(k, v) ((k)==(*(v)))

static const wabi_word wabi_tag_immediate_limit = wabi_tag_forward;
static const wabi_word wabi_word_value_mask = 0x00FFFFFFFFFFFFFF;
static const wabi_word wabi_word_tag_mask = 0xFF00000000000000;
static const int wabi_word_tag_offset = 64 - 8;

#define wabi_sizeof(str) (sizeof(str) >> 3)

static inline char*
wabi_tag_to_string(wabi_word* w)
{
  switch(WABI_TAG(w)) {
  case wabi_tag_atom:
    return "atom";
  case wabi_tag_fixnum:
    return "fixnum";
  case wabi_tag_symbol:
    return "symbol";
  case wabi_tag_forward:
    return "forward";
  case wabi_tag_pair:
    return "pair";
  case wabi_tag_bin_blob:
    return "bin_blob";
  case wabi_tag_bin_leaf:
    return "bin_leaf";
  case wabi_tag_bin_node:
    return "bin_node";
  case wabi_tag_map_array:
    return "map_array";
  case wabi_tag_map_entry:
    return "map_entry";
  case wabi_tag_map_hash:
    return "map_hash";
  case wabi_tag_app:
    return "app";
  case wabi_tag_bt_app:
    return "bt_app";
  case wabi_tag_oper:
    return "oper";
  case wabi_tag_bt_oper:
    return "bt_oper";
  case wabi_tag_env:
    return "env";
  case wabi_tag_place:
    return "place";
  case wabi_tag_alien:
    return "alien";
  case wabi_tag_tagged:
    return "ragged";
  case wabi_tag_cont_eval:
    return "cont_eval";
  case wabi_tag_cont_apply:
    return "cont_apply";
  case wabi_tag_cont_call:
    return "cont_call";
  case wabi_tag_cont_sel:
    return "cont_sel";
  case wabi_tag_cont_args:
    return "cont_args";
  case wabi_tag_cont_def:
    return "cont_def";
  case wabi_tag_cont_prog:
    return "cont_prog";
  case wabi_tag_ct:
    return "continuation";
  case wabi_tag_vector_digit:
    return "vector_digit";
  case wabi_tag_vector_deep:
    return "vector_deep";
  default:
    return "unwknown";
  }
}

#endif
