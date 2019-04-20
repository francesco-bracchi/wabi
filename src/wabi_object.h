/**
 * Memory object layout
 */

#ifndef wabi_object_h

#define wabi_object_h

#include <stdint.h>

typedef uint64_t wabi_word_t;
typedef wabi_word_t wabi_size_t;
typedef wabi_word_t* wabi_obj;
typedef int wabi_error;

#define WABI_WORD_SIZE sizeof(wabi_word_t)

// todo: compact all the constant values (ign, true, false, nil)
// as the same tag (is it feasible? affects the falsey thingie?

#define WABI_TAG_NIL        0x0000000000000000
#define WABI_TAG_BOOL       0x1000000000000000
#define WABI_TAG_SMALLINT   0x2000000000000000
#define WABI_TAG_IGNORE     0x2100000000000000
#define WABI_TAG_FORWARD    0x3000000000000000
#define WABI_TAG_BIN_BLOB   0x4000000000000000
#define WABI_TAG_BIN_LEAF   0x5000000000000000
#define WABI_TAG_BIN_NODE   0x6000000000000000
#define WABI_TAG_SYMBOL     0x7000000000000000
#define WABI_TAG_PAIR       0x8000000000000000
#define WABI_TAG_MAP_ARRAY  0x9000000000000000
#define WABI_TAG_MAP_ENTRY  0x9100000000000000
#define WABI_TAG_MAP_HASH   0x9200000000000000

#define WABI_VALUE_MASK     0x00FFFFFFFFFFFFFF
#define WABI_TAG_MASK       0xFF00000000000000

#define WABI_TAG_ATOMIC_LIMIT WABI_TAG_FORWARD

#define wabi_word_tag(word) ((word) & WABI_TAG_MASK)
#define wabi_word_value(word) ((word) & WABI_VALUE_MASK)
#define wabi_word_is_type(word, type) (wabi_word_tag(word) == type)

#define wabi_obj_tag(obj) wabi_word_tag(*obj)
#define wabi_obj_value(obj) wabi_word_value(*obj)
#define wabi_obj_is_type(obj, type) wabi_word_is_type(*obj, type)

#define wabi_obj_is_nil(obj) wabi_obj_is_type(obj, WABI_TAG_NIL)
#define wabi_obj_is_bool(obj) wabi_obj_is_type(obj, WABI_TAG_BOOL)
#define wabi_obj_is_smallint(obj) wabi_obj_is_type(obj, WABI_TAG_SMALLINT)
#define wabi_obj_is_ignore(obj) wabi_obj_is_type(obj, WABI_TAG_IGNORE)
#define wabi_obj_is_forward(obj) wabi_obj_is_type(obj, WABI_TAG_FORWARD)
#define wabi_obj_is_bin_blob(obj) wabi_obj_is_type(obj, WABI_TAG_BIN_BLOB)
#define wabi_obj_is_bin_leaf(obj) wabi_obj_is_type(obj, WABI_TAG_BIN_LEAF)
#define wabi_obj_is_bin_node(obj) wabi_obj_is_type(obj, WABI_TAG_BIN_NODE)
#define wabi_obj_is_bin(obj) ((wabi_obj_is_bin_leaf(obj)) || (wabi_obj_is_bin_node(obj)))
#define wabi_obj_is_pair(obj) wabi_obj_is_type(obj, WABI_TAG_PAIR)

#define wabi_obj_is_map_array(obj) wabi_obj_is_type(obj, WABI_TAG_MAP_ARRAY)
#define wabi_obj_is_map_hash(obj) wabi_obj_is_type(obj, WABI_TAG_MAP_HASH)
#define wabi_obj_is_map_entry(obj) wabi_obj_is_type(obj, WABI_TAG_MAP_ENTRY)

#define wabi_obj_is_map(obj) ((wabi_obj_is_map_array(obj)) || (wabi_obj_is_map_hash(obj)))
#define wabi_obj_is_symbol(obj) wabi_obj_is_type(obj, WABI_TAG_SYMBOL)

#define wabi_obj_is_atomic(obj) (wabi_obj_tag(obj) < WABI_TAG_ATOMIC_LIMIT)

wabi_size_t
wabi_obj_size(wabi_obj obj);

#endif
