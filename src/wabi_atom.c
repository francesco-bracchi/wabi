#define wabi_atom_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_atom.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_cmp.h"

static inline void
wabi_atom_set_atbl(const wabi_vm vm,
                     const wabi_val binref,
                     const wabi_val sym) {
  wabi_map tbl;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->atbl, binref, sym);
  if(vm->ert) return;
  vm->atbl = (wabi_val) tbl;
}

wabi_atom
wabi_atom_new(const wabi_vm vm,
              const wabi_val binref)
{
  wabi_atom res;
  wabi_map tbl;

  res = (wabi_atom) wabi_map_get((wabi_map) vm->atbl, binref);
  if(res) return res;

  res = (wabi_atom) wabi_vm_alloc(vm, WABI_ATOM_SIZE);
  if(vm->ert) return NULL;

  *res = (wabi_word) binref;
  WABI_SET_TAG(res, wabi_tag_atom);

  tbl = wabi_map_assoc(vm, (wabi_map) vm->atbl, binref, res);
  if(vm->ert) return NULL;
  vm->atbl = (wabi_val) tbl;

  return res;
}
