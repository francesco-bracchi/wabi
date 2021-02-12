#define wabi_atom_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_atom.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

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
  res = (wabi_atom) wabi_map_get((wabi_map) vm->atbl, binref);
  if(res) return res;

  res = (wabi_atom) wabi_vm_alloc(vm, WABI_ATOM_SIZE);
  if(vm->ert) return NULL;

  *res = (wabi_word) binref;
  WABI_SET_TAG(res, wabi_tag_atom);
  wabi_atom_set_atbl(vm, binref, res);
  return res;
}

void
wabi_atom_collect_val(const wabi_vm vm, const wabi_val sym)
{
  wabi_val binref;
  binref = wabi_atom_to_binary(sym);
  binref = wabi_copy_val(vm, binref);
  *sym = (wabi_word) binref;
  WABI_SET_TAG(sym, wabi_tag_atom);
  vm->stor.scan+= WABI_ATOM_SIZE;

  if(wabi_map_get((wabi_map) vm->atbl, binref))
    return;

  wabi_atom_set_atbl(vm, binref, sym);
}


static void
wabi_atom_atom_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_atom);
}


static void
wabi_atom_atom_table(const wabi_vm vm)
{
  vm->ctrl = vm->atbl;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

static void
wabi_atom_atom(const wabi_vm vm)
{
  wabi_val ctrl, bin, res;

  ctrl = vm->ctrl;

  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  bin = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_atom_is_nil(vm, bin)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
    return;
  }
  if (!wabi_is_binary(bin)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  res = wabi_atom_new(vm, bin);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

void
wabi_atom_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "atom?", &wabi_atom_atom_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "atom-table", &wabi_atom_atom_table);
  if(vm->ert) return;
  wabi_defn(vm, env, "atom", &wabi_atom_atom);
}
