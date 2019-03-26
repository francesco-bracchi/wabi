#ifndef wabi_hash_h

#define wabi_hash_h

#include "wabi_object.h"
#include "wabi_err.h"


typedef struct wabi_hash_state_struct
{
  wabi_word_t a;
  wabi_word_t b;
  wabi_word_t v_hash;
} wabi_hash_state_t;


void
wabi_hash(wabi_obj obj, wabi_obj *res, wabi_error* err);


wabi_word_t
wabi_hash_raw(wabi_obj obj);

#endif
