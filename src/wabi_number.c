#define wabi_atomic_c

#include <stdint.h>
#include <stdio.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_atom.h"
#include "wabi_number.h"
#include "wabi_combiner.h"
#include "wabi_builtin.h"
#include "wabi_error.h"
#include "wabi_list.h"

wabi_fixnum
wabi_fixnum_new(const wabi_vm vm,
                const int64_t val)
{
  wabi_val res;

  if(val >= wabi_fixnum_max) {
    vm->ert = wabi_error_out_of_range;
    return NULL;
  }
  res = wabi_vm_alloc(vm, 1);
  if(vm->ert) return NULL;

  *res = val & wabi_word_value_mask;
  WABI_SET_TAG(res, wabi_tag_fixnum);
  return res;
}
