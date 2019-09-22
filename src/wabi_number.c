#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_number.h"

wabi_fixnum
wabi_fixnum_new(wabi_vm vm,
                int64_t val)
{
  wabi_val res = wabi_vm_alloc(vm, 1);
  if(res) {
    *res = val & wabi_word_value_mask;
    WABI_SET_TAG(res, wabi_tag_fixnum);
    return res;
  }
  return NULL;
}
