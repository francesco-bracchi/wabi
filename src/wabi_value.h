/**
 * Memory value layout
 */

#ifndef wabi_value_h

#define wabi_value_h

#include <stdint.h>

typedef uint64_t wabi_word_t;
typedef wabi_word_t wabi_size_t;
typedef wabi_word_t* wabi_val;
typedef int wabi_error;

#define WABI_WORD_SIZE sizeof(wabi_word_t)

// todo: compact all the constant values (ign, true, false, nil)
// as the same tag (is it feasible? affects the falsey thingie?

#define WABI_TAG_NIL         0x0000000000000000
#define WABI_TAG_BOOL        0x1000000000000000
#define WABI_TAG_SMALLINT    0x2000000000000000
#define WABI_TAG_IGNORE      0x3000000000000000
#define WABI_TAG_FORWARD     0x4000000000000000
#define WABI_TYPE_SYMBOL     0x5000000000000000
#define WABI_TAG_SYMBOL      0x5000000000000000
#define WABI_TYPE_BIN        0x6000000000000000
#define WABI_TAG_BIN_BLOB    0x6000000000000000
#define WABI_TAG_BIN_LEAF    0x6100000000000000
#define WABI_TAG_BIN_NODE    0x6200000000000000
#define WABI_TYPE_PAIR       0x7000000000000000
#define WABI_TAG_PAIR        0x7000000000000000
#define WABI_TYPE_MAP        0x8000000000000000
#define WABI_TAG_MAP_ARRAY   0x8000000000000000
#define WABI_TAG_MAP_ENTRY   0x8100000000000000
#define WABI_TAG_MAP_HASH    0x8200000000000000
#define WABI_TYPE_VERB       0x9000000000000000
#define WABI_TAG_OPERATIVE   0x9100000000000000
#define WABI_TAG_APPLICATIVE 0x9200000000000000
#define WABI_TAG_BUILTIN     0x9300000000000000

#define WABI_VALUE_MASK      0x00FFFFFFFFFFFFFF
#define WABI_TAG_MASK        0xFF00000000000000
#define WABI_TYPE_MASK       0xF000000000000000

#define WABI_TAG_ATOMIC_LIMIT WABI_TAG_FORWARD

#define wabi_word_tag(word) ((word) & WABI_TAG_MASK)
#define wabi_word_type(word) ((word) & WABI_TYPE_MASK)
#define wabi_word_value(word) ((word) & WABI_VALUE_MASK)
#define wabi_word_is_tagged(word, tag) (wabi_word_tag(word) == tag)
#define wabi_word_is_type(word, type) (wabi_word_type(word) == type)

#define wabi_val_tag(val) wabi_word_tag(*val)
#define wabi_val_type(val) wabi_word_type(*val)
#define wabi_val_value(val) wabi_word_value(*val)
#define wabi_val_is_tagged(val, tag) wabi_word_is_tagged(*val, tag)
#define wabi_val_is_type(val, type) wabi_word_is_type(*val, type)

#define wabi_val_is_nil(val) wabi_val_is_tagged(val, WABI_TAG_NIL)
#define wabi_val_is_bool(val) wabi_val_is_tagged(val, WABI_TAG_BOOL)
#define wabi_val_is_smallint(val) wabi_val_is_tagged(val, WABI_TAG_SMALLINT)
#define wabi_val_is_ignore(val) wabi_val_is_tagged(val, WABI_TAG_IGNORE)
#define wabi_val_is_forward(val) wabi_val_is_tagged(val, WABI_TAG_FORWARD)
#define wabi_val_is_atomic(val) (wabi_val_tag(val) < WABI_TAG_ATOMIC_LIMIT)

#define wabi_val_is_bin_blob(val) wabi_val_is_tagged(val, WABI_TAG_BIN_BLOB)
#define wabi_val_is_bin_leaf(val) wabi_val_is_tagged(val, WABI_TAG_BIN_LEAF)
#define wabi_val_is_bin_node(val) wabi_val_is_tagged(val, WABI_TAG_BIN_NODE)
#define wabi_val_is_bin(val) wabi_val_is_type(val, WABI_TYPE_BIN)

#define wabi_val_is_pair(val) wabi_val_is_tagged(val, WABI_TAG_PAIR)

#define wabi_val_is_map_array(val) wabi_val_is_tagged(val, WABI_TAG_MAP_ARRAY)
#define wabi_val_is_map_hash(val) wabi_val_is_tagged(val, WABI_TAG_MAP_HASH)
#define wabi_val_is_map_entry(val) wabi_val_is_tagged(val, WABI_TAG_MAP_ENTRY)
#define wabi_val_is_map(val) wabi_val_is_type(val, WABI_TYPE_MAP)

#define wabi_val_is_symbol(val) wabi_val_is_tagged(val, WABI_TAG_SYMBOL)

#endif
