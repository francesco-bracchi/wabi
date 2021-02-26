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
#include "wabi_error.h"


wabi_combiner
wabi_operator_builtin_new(const wabi_vm vm,
                          const wabi_binary cname,
                          const wabi_builtin_fun cfun)
{
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  res->c_xtra = (wabi_word) vm->nil;
  WABI_SET_TAG(res, wabi_tag_bt_oper);

  return (wabi_combiner) res;
}

wabi_combiner
wabi_application_builtin_new(const wabi_vm vm,
                             const wabi_binary cname,
                             const wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
  if(vm->ert) return NULL;
  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  res->c_xtra = (wabi_word) vm->nil;
  WABI_SET_TAG(res, wabi_tag_bt_app);
  return (wabi_combiner) res;
}

wabi_combiner
wabi_combiner_continuation_new(const wabi_vm vm, const wabi_cont cont)
{
  wabi_combiner_continuation res;
  res = (wabi_combiner_continuation) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
  if(vm->ert) return NULL;
  res->cont = (wabi_word) cont;
  WABI_SET_TAG(res, wabi_tag_ct);
  return (wabi_combiner) res;
}
