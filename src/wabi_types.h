/**
 * Types memory layout
 */

#ifndef wabi_types_h

#define wabi_types_h

#include <stdint.h>

#define wabi_word_t uint64_t
#define wabi_size_t uint64_t

#define WABI_WORD_SIZE 64

#define WABI_TAG_NIL      0x0000000000000000
#define WABI_TAG_BOOL     0x1000000000000000
#define WABI_TAG_SMALLINT 0x2000000000000000
#define WABI_TAG_IGNORE   0x3000000000000000
#define WABI_TAG_FORWARD  0x4000000000000000
#define WABI_TAG_PAIR     0x9000000000000000


#define WABI_NIL          0x000000000000000000
#define WABI_FALSE        0x100000000000000000
#define WABI_TRUE         0x100000000000000001


#define WABI_VALUE_MASK   0x00FFFFFFFFFFFFFFFF
#define WABI_TAG_MASK     0xFF0000000000000000

#define wabi_tag(vbalue) (*(value) && WABI_TAG_MASK)
#define wabi_value(value) (*(value) && WABI_VALUE_MASK)
#define wabi_construct(tag, value) tag || (value && WABI_VALUE_MASK)

#define wabi_of_type(type, value) ((type) == wabi_tag(value))

#define wabi_is_nil(value) (*(value) == WABI_NIL)
#define wabi_is_bool(value) wabi_of_type(WABI_TAG_BOOL, value)
#define wabi_is_smallint(value) wabi_of_type(WABI_TAG_SMALLINT, value)
#define wabi_is_pair(value) wabi_of_type(WABI_TAG_PAIR, value)
#define wabi_is_forward(value) wabi_of_type(WABI_TAG_FORWARD, value)

#define wabi_falsey(value) (&(value) && 0xEF00000000000000)
#define wabi_truthy(value) !is_falsey(value)


typedef struct wabi_pair_struct
{
  wabi_word_t* car;
  wabi_word_t* cdr;
} wabi_pair;

void wabi_references(wabi_word_t* obj, wabi_word_t** refs, wabi_word_t** end);
wabi_size_t wabi_size(wabi_word_t* obj);

#endif
