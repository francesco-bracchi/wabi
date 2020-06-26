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
wabi_hash_val(const wabi_hash_state state, const wabi_val val);


static inline void
wabi_hash_step(const wabi_hash_state state,
               const char *data,
               const wabi_word size)
{
  wabi_word j;
  for(j = 0; j < size; j++) {
    state->v_hash = state->a * state->v_hash + *(data + j);
    state->a *= state->b;
  }
}

#endif
