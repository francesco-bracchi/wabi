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
wabi_place_new(const wabi_vm vm,
               const wabi_val init);


void
wabi_place_builtins(const wabi_vm vm,
                    const wabi_env env);


static inline wabi_val
wabi_place_val(const wabi_place place)
{
  return (wabi_val) place->val;
}

static inline void
wabi_place_val_set(const wabi_place place,
                   const wabi_val val)
{
  place->val = (wabi_word) val;
}


static inline void
wabi_place_copy_val(const wabi_vm vm,
                    const wabi_place place)
{
  wabi_copy_val_size(vm, (wabi_val) place, WABI_PLACE_SIZE);
}


static inline void
wabi_place_collect_val(const wabi_vm vm,
                       const wabi_place place)
{
  wabi_place_val_set(place, wabi_copy_val(vm, wabi_place_val(place)));
  vm->stor.scan+= WABI_PLACE_SIZE;
}


static inline void
wabi_place_hash(const wabi_hash_state state,
                const wabi_place place)
{
  wabi_hash_step(state, "R", 1);
  wabi_hash_step(state, (char*) &(place->uid), WABI_WORD_SIZE);
}


static inline int
wabi_place_cmp(const wabi_place left,
               const wabi_place right)
{
  if(left->uid == right->uid) return 0;
  if(left->uid > right->uid) return 1;
  return -1;
}

static inline int
wabi_is_place(const wabi_val val) {
  return WABI_IS(wabi_tag_place, val);
}

#endif
