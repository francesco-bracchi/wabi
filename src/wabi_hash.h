#ifndef wabi_hash_h

#define wabi_hash_h

#include "wabi_value.h"

typedef struct wabi_hash_state_struct
{
  wabi_word a;
  wabi_word b;
  wabi_word v_hash;
  int err;
} wabi_hash_state_t;

typedef wabi_hash_state_t* wabi_hash_state;

wabi_word
wabi_hash(wabi_val val);

void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val);

void
wabi_hash_step(wabi_hash_state state, char *data, wabi_word size);

#endif
