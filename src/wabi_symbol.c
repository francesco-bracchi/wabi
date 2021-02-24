#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

wabi_symbol
wabi_symbol_new(const wabi_vm vm,
                const wabi_val binref)
{
  wabi_symbol res;
  wabi_map tbl;

  res = (wabi_symbol) wabi_map_get((wabi_map) vm->stbl, binref);
  if(res) return res;

  res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
  if(vm->ert) return NULL;

  *res = (wabi_word) binref;
  WABI_SET_TAG(res, wabi_tag_symbol);

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, res);
  if(vm->ert) return NULL;
  vm->stbl = (wabi_val) tbl;

  return res;
}



static void
wabi_symbol_sym_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_symbol);
}


/* static void */
/* wabi_symbol_symbol_table(const wabi_vm vm) */
/* { */
/*   vm->ctrl = vm->stbl; */
/*   vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont); */
/* } */

static void
wabi_symbol_sym(const wabi_vm vm)
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
  res = wabi_symbol_new(vm, bin);
  if (vm->ert)
    return;
  vm->ctrl = res;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

void
wabi_symbol_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "sym?", &wabi_symbol_sym_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "sym", &wabi_symbol_sym);
}
