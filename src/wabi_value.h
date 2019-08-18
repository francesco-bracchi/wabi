#ifndef wabi_value_h

#define wabi_value_h

#include "wabi_word.h"

typedef int wabi_error_code;

typedef enum wabi_tag_enum {
  wabi_tag_constant  = 0x00,
  wabi_tag_integer   = 0x01,
  wabi_tag_rational  = 0x02,
  wabi_tag_float     = 0x03,
  wabi_tag_forward   = 0x04,
  wabi_tag_symbol    = 0x05,
  wabi_tag_pair      = 0x06,

  wabi_tag_bin_blob  = 0x07,
  wabi_tag_bin_leaf  = 0x08,
  wabi_tag_bin_node  = 0x09,

  wabi_tag_map_array = 0x0A,
  wabi_tag_map_entry = 0x0B,
  wabi_tag_map_hash  = 0x0C,
  wabi_tag_map_operb = 0x0D,

  wabi_tag_oper      = 0x0E,
  wabi_tag_app       = 0x0F,
  wabi_tag_bt_oper   = 0x10,
  wabi_tag_bt_app    = 0x11,

  wabi_tag_env       = 0x12,
  wabi_tag_var       = 0x13
} wabi_tag;

static const wabi_word wabi_val_nil    = 0x0000000000000000;
static const wabi_word wabi_val_false  = 0x0000000000000001;
static const wabi_word wabi_val_true   = 0x0000000000000002;
static const wabi_word wabi_val_ignore = 0x0000000000000003;

typedef wabi_word* wabi_val;

#define WABI_TAG_ATOMIC_LIMIT wabi_tag_enum.wabi_tag_forward

#define WABI_WORD_MASK 0x07FFFFFFFFFFFFFF

#define WABI_WORD_TAG(word) ((word) >> 59)

#define WABI_WORD_VAL(word) ((word) & WABI_WORD_MASK)

#define WABI_TAG(val) WABI_WORD_TAG(*val)

#define WABI_DEREF(val) ((wabi_val) WABI_WORD_VAL(*val))

#define WABI_VAL_IS_TYPE(type, val) (WABI_TAG(val) == (type))

#endif
