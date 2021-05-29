#define wabi_combiner_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_builtin.h"
#include "wabi_symbol.h"
#include "wabi_atom.h"
#include "wabi_meta.h"
#include "wabi_error.h"


wabi_combiner
wabi_operator_builtin_new(const wabi_vm vm,
                          const wabi_word bid)
{
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;
  res->bid = bid;
  WABI_SET_TAG(res, wabi_tag_bt_oper);

  return (wabi_combiner) res;
}

wabi_combiner
wabi_application_builtin_new(const wabi_vm vm,
                             const wabi_word bid)
{
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;
  res->bid = bid;
  WABI_SET_TAG(res, wabi_tag_bt_app);
  return (wabi_combiner) res;
}

wabi_combiner
wabi_combiner_continuation_new(const wabi_vm vm, const wabi_cont cont, const wabi_meta atem)
{
  wabi_combiner_continuation res;
  res = (wabi_combiner_continuation) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
  if(vm->ert) return NULL;
  res->cont = (wabi_word) cont;
  res->atem = (wabi_word) atem;
  WABI_SET_TAG(res, wabi_tag_ct);
  return (wabi_combiner) res;
}
