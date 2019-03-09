#define wabi_object_c

#include "wabi_object.h"
#include "wabi_pair.h"

inline wabi_size_t
wabi_obj_size(wabi_obj obj)
{
  switch(wabi_obj_tag(obj)) {
  case WABI_TAG_PAIR:
    return WABI_PAIR_SIZE;
  default:
    return 1;
  }
}
