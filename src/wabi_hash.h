#ifndef wabi_hash_h

#define wabi_hash_h

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"


typedef struct wabi_hash_state_struct
{
  wabi_word_t a;
  wabi_word_t b;
  wabi_word_t v_hash;
} wabi_hash_state_t;


wabi_val
wabi_hash(wabi_vm vm, wabi_val val);


wabi_word_t
wabi_hash_raw(wabi_val val);

#endif
