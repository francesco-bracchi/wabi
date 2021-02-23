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

void
wabi_cont_concat_cont(const wabi_vm vm,
                      const wabi_cont cont0)
{
  wabi_cont cont, res_cont, prev_cont, new_prev_cont, new_cont, right_cont;
  wabi_cont_prompt res_prompt, new_prompt, prev_new_prompt, right_prompt;

  wabi_val ctrl, fst;

  ctrl = vm->ctrl;
  cont = cont0;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  fst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if (*((wabi_val)cont) == 0) {
    vm->ctrl = fst;
    vm->env = vm->nil;
    vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
    return;
  }
  new_cont = (wabi_cont) wabi_cont_copy(vm, cont);
  if(vm->ert) return;

  res_cont = new_cont;
  res_prompt = (wabi_cont_prompt) wabi_cont_done;
  new_prompt = (wabi_cont_prompt) wabi_cont_done;
  for(;;) {
    if (*((wabi_val)wabi_cont_pop(cont)) == 0) {
      break;
    }
    prev_cont = cont;
    new_prev_cont = new_cont;
    cont = wabi_cont_pop(cont);
    new_cont = wabi_cont_copy(vm, cont);
    if(vm->ert) return;
    if(new_prev_cont) new_prev_cont->next = ((wabi_word) new_cont) | WABI_TAG(prev_cont);

    if(WABI_IS(wabi_tag_cont_prompt, cont)) {
      prev_new_prompt = new_prompt;
      new_prompt = (wabi_cont_prompt) new_cont;
      if(prev_new_prompt) prev_new_prompt->next_prompt = (wabi_word) new_prompt;
      if(! res_prompt) res_prompt = new_prompt;
    }
  }
  right_cont = wabi_cont_pop((wabi_cont) vm->cont);
  right_prompt = (wabi_cont_prompt) vm->prmt;
  new_cont->next = (wabi_word) right_cont | WABI_TAG(new_cont);

  vm->ctrl = fst;
  vm->env = vm->nil;
  vm->cont = (wabi_val) res_cont;
  if(new_prompt) {
    new_prompt->next_prompt = (wabi_word) right_prompt;
    vm->prmt = (wabi_val) res_prompt;
  }
}
