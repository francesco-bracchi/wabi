#include "wabi_value.h"
#include "wabi_cont.h"
#include "wabi_vm.h"
#include "wabi_combiner.h"
#include "wabi_atom.h"

#include "wabi_pr.h"
#include <stdio.h>

#define wabi_cont_c

static inline wabi_size
wabi_cont_size(const wabi_cont cont)
{
  switch(WABI_TAG(cont)) {
  case wabi_tag_cont_eval:
    return WABI_CONT_EVAL_SIZE;
  case wabi_tag_cont_prompt:
    return WABI_CONT_PROMPT_SIZE;
  case wabi_tag_cont_apply:
    return WABI_CONT_APPLY_SIZE;
  case wabi_tag_cont_call:
    return WABI_CONT_CALL_SIZE;
  case wabi_tag_cont_sel:
    return WABI_CONT_SEL_SIZE;
  case wabi_tag_cont_args:
    return WABI_CONT_ARGS_SIZE;
  case wabi_tag_cont_def:
    return WABI_CONT_DEF_SIZE;
  case wabi_tag_cont_prog:
    return WABI_CONT_PROG_SIZE;
  default:
    return -1;
  }
}

void
wabi_cont_copy_val(const wabi_store store, const wabi_cont cont)
{
  wabi_size size = wabi_cont_size(cont);
  wordcopy(store->heap, (wabi_word*) cont, size);
  store->heap += size;
}

static inline wabi_cont
wabi_cont_copy(const wabi_vm vm, const wabi_cont cont) {
  wabi_size size;
  wabi_cont res;
  size = wabi_cont_size(cont);
  res = (wabi_cont) wabi_vm_alloc(vm, size);
  if(vm->ert) return NULL;

  wordcopy((wabi_word*) res, (wabi_word*) cont, size);
  return res;
}
