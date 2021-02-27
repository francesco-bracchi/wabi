#ifndef wabi_place_h

#define wabi_place_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"

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

static inline int
wabi_is_place(const wabi_val val) {
  return WABI_IS(wabi_tag_place, val);
}

#endif
