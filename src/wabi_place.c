#define wabi_place_c

#include <stdio.h>

#include "wabi_place.h"
#include "wabi_error.h"
#include "wabi_env.h"
#include "wabi_list.h"
#include "wabi_atom.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"


static inline wabi_word
wabi_place_uid(const wabi_place place)
{
  static wabi_word wabi_place_cnt = 0;
  return ((wabi_word) place) ^ (++wabi_place_cnt);
}


wabi_place
wabi_place_new(const wabi_vm vm,
               const wabi_val init)
{
  wabi_place place = (wabi_place) wabi_vm_alloc(vm, WABI_PLACE_SIZE);
  if(vm->ert) return NULL;

  place->uid = wabi_place_uid(place);
  place->val = (wabi_word) init;
  WABI_SET_TAG(place, wabi_tag_place);
  return place;
}
