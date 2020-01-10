#ifndef wabi_value_h

#define wabi_value_h

#include "wabi_word.h"

typedef int wabi_error_code;

typedef enum wabi_tag_enum {
  wabi_tag_constant       = 0x0000000000000000,
  wabi_tag_fixnum         = 0x0800000000000000,
  wabi_tag_symbol         = 0x1000000000000000,
  wabi_tag_forward        = 0x1800000000000000,
  wabi_tag_pair           = 0x2000000000000000,

  wabi_tag_bin_blob       = 0x2800000000000000,
  wabi_tag_bin_leaf       = 0x3000000000000000,
  wabi_tag_bin_node       = 0x3800000000000000,

  wabi_tag_map_array      = 0x4000000000000000,
  wabi_tag_map_entry      = 0x4800000000000000,
  wabi_tag_map_hash       = 0x5000000000000000,

  wabi_tag_app            = 0x5800000000000000, // 0101 1
  wabi_tag_bt_app         = 0x6000000000000000, // 0110 0
  wabi_tag_oper           = 0x6800000000000000, // 0110 1
  wabi_tag_bt_oper        = 0x7000000000000000, // 0111 0

  wabi_tag_env            = 0x7800000000000000,

  wabi_tag_var            = 0x8000000000000000,
  wabi_tag_alien          = 0x8800000000000000,
  wabi_tag_tagged         = 0x9000000000000000,

  wabi_tag_cont_eval      = 0x9000000000000000,
  wabi_tag_cont_prompt    = 0x9800000000000000,
  wabi_tag_cont_apply     = 0xa000000000000000,
  wabi_tag_cont_call      = 0xa800000000000000,
  wabi_tag_cont_sel       = 0xb000000000000000,
  wabi_tag_cont_eval_more = 0xb800000000000000,
  wabi_tag_cont_def       = 0xc000000000000000,
  wabi_tag_cont_prog      = 0xc800000000000000,

  wabi_tag_control        = 0xd000000000000000
} wabi_tag;

typedef wabi_word* wabi_val;

typedef wabi_val wabi_expr;

#define WABI_WORD_TAG(word) ((word) & wabi_word_tag_mask)

#define WABI_WORD_VAL(word) ((word) & wabi_word_value_mask)

#define WABI_TAG(val) WABI_WORD_TAG(*((wabi_val) val))

#define WABI_DEREF(val) ((wabi_val) WABI_WORD_VAL(*val))

#define WABI_SET_TAG(val,tag) (*((wabi_word *) val) |= tag)

#define WABI_IS(tag, val) (WABI_TAG((wabi_word *) val) == tag)

static const wabi_word wabi_val_nil    = 0x0000000000000000;
static const wabi_word wabi_val_false  = 0x0000000000000001;
static const wabi_word wabi_val_true   = 0x0000000000000002;
static const wabi_word wabi_val_ignore = 0x0000000000000003;
static const wabi_word wabi_val_zero   = wabi_tag_fixnum;

#define WABI_IS_VAL(k, v) ((k)==(*(v)))

static const wabi_word wabi_tag_immediate_limit = wabi_tag_forward;
static const wabi_word wabi_word_value_mask = 0x07FFFFFFFFFFFFFF;
static const wabi_word wabi_word_tag_mask = 0xF800000000000000;
static const int wabi_word_tag_offset = 64 - 5;

#endif
