#define wabi_types_c

#include "wabi_types.h"

wabi_size_t wabi_type_size(wabi_word_t* obj)
{
  wabi_word_t tag = wabi_tag(obj);

  if(tag <= WABI_TYPE_TAG_FORWARD)
    return 1;

  if(tag == WABI_TYPE_TAG_PAIR)
    return 2;

  if(tag == WABI_TYPE_TAG_BINARY)
    return *obj + 1;

  return 0;
}
