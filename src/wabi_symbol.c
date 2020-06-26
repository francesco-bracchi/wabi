#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

static inline void
wabi_symbol_set_stbl(const wabi_vm vm,
                     const wabi_val binref,
                     const wabi_val sym) {
  wabi_map tbl;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, sym);
  if(vm->ert) return;
  vm->stbl = (wabi_val) tbl;
}

wabi_symbol
wabi_symbol_new(const wabi_vm vm,
                const wabi_val binref)
{
  wabi_symbol res;
  wabi_val new_table;
  res = (wabi_symbol) wabi_map_get((wabi_map) vm->stbl, binref);
  if(res) return res;

  res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
  if(vm->ert) return NULL;

  *res = (wabi_word) binref;
  WABI_SET_TAG(res, wabi_tag_symbol);
  wabi_symbol_set_stbl(vm, binref, res);
  return res;
}


void
wabi_symbol_collect_val(const wabi_vm vm, const wabi_val sym)
{
  wabi_val binref, sym0;
  binref = wabi_symbol_to_binary(sym);
  binref = wabi_copy_val(vm, binref);
  *sym = (wabi_word) binref;
  WABI_SET_TAG(sym, wabi_tag_symbol);
  vm->stor.scan+= WABI_SYMBOL_SIZE;

  if(wabi_map_get((wabi_map) vm->stbl, binref))
    return;

  wabi_symbol_set_stbl(vm, binref, sym);
}


static void
wabi_symbol_sym_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_symbol);
}


void
wabi_symbol_symbol_table(const wabi_vm vm)
{
  vm->ctrl = vm->stbl;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


void
wabi_symbol_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_error_type res;
  WABI_DEFN(vm, env, "sym?", "sym?", wabi_symbol_sym_p);
  if(vm->ert) return;
  WABI_DEFN(vm, env, "symbol-table", "symbol-table", wabi_symbol_symbol_table);
}
