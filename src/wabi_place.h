#ifndef wabi_place_h

#define wabi_place_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"
#include "wabi_collect.h"
#include "wabi_hash.h"
#include "wabi_cmp.h"

typedef struct wabi_place_struct
{
  wabi_word uid;
  wabi_word val;
} wabi_place_t;

typedef wabi_place_t* wabi_place;

#define WABI_PLACE_SIZE wabi_sizeof(wabi_place_t)


wabi_place
wabi_place_new(wabi_vm vm, wabi_val init);


wabi_error_type
wabi_place_builtins(wabi_vm vm, wabi_env env);


static inline wabi_val
wabi_place_val(wabi_place place)
{
  return (wabi_val) place->val;
}

static inline void
wabi_place_val_set(wabi_place place, wabi_val val)
{
  place->val = (wabi_word) val;
}


static inline void
wabi_place_copy_val(wabi_vm vm, wabi_place place)
{
  wabi_copy_val_size(vm, (wabi_val) place, WABI_PLACE_SIZE);
}


static inline void
wabi_place_collect_val(wabi_vm vm, wabi_place place)
{
  wabi_place_val_set(place, wabi_copy_val(vm, wabi_place_val(place)));
  vm->stor.scan+= WABI_PLACE_SIZE;
}


static inline void
wabi_place_hash(wabi_hash_state state, wabi_place place) {
  wabi_hash_step(state, "R", 1);
  wabi_hash_step(state, (char*) &(place->uid), WABI_WORD_SIZE);
}


static inline int
wabi_place_cmp(wabi_place left, wabi_place right)
{
  if(left->uid == right->uid) return 0;
  if(left->uid > right->uid) return 1;
  return -1;
}

#endif
