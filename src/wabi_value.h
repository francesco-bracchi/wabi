#ifndef wabi_value_h

#define wabi_value_h

#include "wabi_word.h"

typedef int wabi_error_code;

typedef enum wabi_tag_enum {
  wabi_tag_constant   = 0x0000000000000000,
  wabi_tag_integer    = 0x0000000000000000,
  wabi_tag_rational   = 0x1000000000000000,
  wabi_tag_float      = 0x2000000000000000,
  wabi_tag_forward    = 0x3000000000000000,
  wabi_tag_symbol     = 0x4000000000000000,
  wabi_tag_pair       = 0x5000000000000000,

  wabi_tag_bin_blob   = 0x6000000000000000,
  wabi_tag_bin_leaf   = 0x7000000000000000,
  wabi_tag_bin_node   = 0x8000000000000000,

  wabi_tag_map_array  = 0x9000000000000000,
  wabi_tag_map_entry  = 0xA000000000000000,
  wabi_tag_map_hash   = 0xB000000000000000,

  wabi_tag_oper       = 0xC000000000000000,
  wabi_tag_app        = 0xD000000000000000,
  wabi_tag_bt_oper    = 0xE000000000000000,
  wabi_tag_bt_app     = 0xF800000000000000,

  wabi_tag_env        = 0x0800000000000000,
  wabi_tag_var        = 0x1800000000000000,
  wabi_tag_cont_eval  = 0x2800000000000000,
  wabi_tag_cont_apply = 0x3800000000000000,
} wabi_tag;

static const wabi_word wabi_val_nil    = 0x0000000000000000;
static const wabi_word wabi_val_false  = 0x0000000000000001;
static const wabi_word wabi_val_true   = 0x0000000000000002;
static const wabi_word wabi_val_ignore = 0x0000000000000003;

static const wabi_word wabi_tag_immediate_limit = wabi_tag_forward;
static const wabi_word wabi_word_value_mask = 0x07FFFFFFFFFFFFFF;
static const wabi_word wabi_word_tag_mask = 0xF800000000000000;
static const int wabi_word_tag_offset = 64 - 5;

typedef wabi_word* wabi_val;

#define WABI_WORD_TAG(word) ((word) & wabi_word_tag_mask)

#define WABI_WORD_VAL(word) ((word) & wabi_word_value_mask)

#define WABI_TAG(val) WABI_WORD_TAG(*((wabi_val) val))

#define WABI_DEREF(val) ((wabi_val) WABI_WORD_VAL(*val))

#define WABI_SET_TAG(val,tag) (*((wabi_word *) val) |= tag)

#define WABI_IS_IMMEDIATE(val) (WABI_TAG(val) < wabi_tag_immediate_limit)

#define WABI_IS(tag, val) (WABI_TAG((wabi_word *) val) == tag)
#endif
