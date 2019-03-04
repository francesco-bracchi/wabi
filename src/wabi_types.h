/**
 * Types memory layout
 */

#ifndef wabi_types_h

#define wabi_types_h

#include <stdint.h>

#define wabi_word_t uint64_t
#define wabi_size_t uint64_t

#define WABI_WORD_SIZE sizeof(wabi_word_t)

#define WABI_TYPE_TAG_NIL       0x0000000000000000
#define WABI_TYPE_TAG_BOOL      0x1000000000000000
#define WABI_TYPE_TAG_SMALLINT  0x2000000000000000
#define WABI_TYPE_TAG_IGNORE    0x3000000000000000
#define WABI_TYPE_TAG_FORWARD   0x4000000000000000
#define WABI_TYPE_TAG_BINARY    0x5000000000000000
#define WABI_TYPE_TAG_PAIR      0xF000000000000000


#define WABI_VALUE_NIL          0x0000000000000000
#define WABI_VALUE_FALSE        0x1000000000000000
#define WABI_VALUE_TRUE         0x1000000000000001


#define WABI_TYPE_VALUE_MASK    0x00FFFFFFFFFFFFFF
#define WABI_TYPE_TAG_MASK      0xFF00000000000000

#define wabi_type_tag(value) (*(value) & WABI_TYPE_TAG_MASK)
#define wabi_type_value(value) (*(value) & WABI_TYPE_VALUE_MASK)
#define wabi_type_tag_value(tag, value) ((tag) | (value))

#define wabi_of_type(type, value) (type == wabi_type_tag(value))

#define wabi_is_nil(value) (*(value) == WABI_VALUE_NIL)
#define wabi_is_bool(value) wabi_of_type(WABI_TYPE_TAG_BOOL, value)
#define wabi_is_smallint(value) wabi_of_type(WABI_TYPE_TAG_SMALLINT, value)
#define wabi_is_binary(value) wabi_of_type(WABI_TYPE_TAG_FORWARD, value)
#define wabi_is_forward(value) wabi_of_type(WABI_TYPE_TAG_FORWARD, value)
#define wabi_is_pair(value) wabi_of_type(WABI_TYPE_TAG_PAIR, va

#define wabi_falsey(value) (&(value) & 0xEF00000000000000)
#define wabi_truthy(value) !is_falsey(value)

#define wabi_forward(value) wabi_tag_value(WABI_TYPE_TAG_FORWARD, value)

typedef struct wabi_pair_struct
{
  wabi_word_t car;
  wabi_word_t cdr;
} wabi_pair_t;

wabi_size_t wabi_type_size(wabi_word_t* obj);

#endif
