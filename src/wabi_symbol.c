#define wabi_symbol_c

#include <stdlib.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_symbol.h"
#include "wabi_map.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"

static inline void
wabi_symbol_set_stbl(wabi_vm vm, wabi_val binref, wabi_val sym) {
  wabi_map tbl;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, sym);
  if(tbl) {
    vm->stbl = (wabi_val) tbl;
  }
}

wabi_symbol
wabi_symbol_new(wabi_vm vm,
                wabi_val binref)
{
  wabi_symbol res;
  wabi_val new_table;
  if(! binref) return NULL;
  res = (wabi_symbol) wabi_map_get((wabi_map) vm->stbl, binref);
  if(res) return res;

  res = (wabi_symbol) wabi_vm_alloc(vm, WABI_SYMBOL_SIZE);
  if(res) {
    *res = (wabi_word) binref;
    WABI_SET_TAG(res, wabi_tag_symbol);
    wabi_symbol_set_stbl(vm, binref, res);
  }
  return res;
}


void
wabi_symbol_collect_val(wabi_vm vm, wabi_val sym)
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


static inline wabi_error_type
wabi_symbol_sym_p_bt(wabi_vm vm, wabi_val e) {
  wabi_val res;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(res) {
    *res = WABI_IS(wabi_tag_symbol, e) ? wabi_val_true : wabi_val_false;
    vm->ctrl = res;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  return wabi_error_nomem;
}

WABI_BUILTIN_WRAP1(wabi_symbol_sym_p, wabi_symbol_sym_p_bt);


wabi_error_type
wabi_symbol_symbol_table(wabi_vm vm)
{
  vm->ctrl = vm->stbl;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_nomem;
}


wabi_error_type
wabi_symbol_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "sym?", "sym?", wabi_symbol_sym_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "symbol-table", "symbol-table", wabi_symbol_symbol_table);
  return res;
}
