#include "wabi_value.h"
#include "wabi_cont.h"
#include "wabi_cmp.h"
#include "wabi_vm.h"
#include "wabi_combiner.h"
#include "wabi_constant.h"

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


/**
 * hashing
 */
void
wabi_cont_hash(const wabi_hash_state state, const wabi_cont cont0)
{
  wabi_cont cont;
  cont = cont0;
  wabi_hash_step(state, "O", 1);
  while(cont) {
    switch(WABI_TAG(cont)) {
    case wabi_tag_cont_eval:
      wabi_hash_step(state, "E", 1);
      break;
    case wabi_tag_cont_prompt:
      wabi_hash_step(state, "P", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prompt) cont)->tag);
      break;
    case wabi_tag_cont_apply:
      wabi_hash_step(state, "A", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_apply) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_apply) cont)->args);
      break;
    case wabi_tag_cont_call:
      wabi_hash_step(state, "C", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_call) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_call) cont)->combiner);
      break;
    case wabi_tag_cont_sel:
      wabi_hash_step(state, "S", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->left);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->right);
      break;
    case wabi_tag_cont_args:
      wabi_hash_step(state, "R", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->data);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->done);
      break;
    case wabi_tag_cont_def:
      wabi_hash_step(state, "D", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_def) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_def) cont)->pattern);
      break;
    case wabi_tag_cont_prog:
      wabi_hash_step(state, "P", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prog) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prog) cont)->expressions);
      break;
    default:
      break;
    }
    cont = wabi_cont_next(cont);
  }
}


void
wabi_cont_concat_cont(const wabi_vm vm, const wabi_cont cont0)
{
  wabi_cont cont, res_cont, prev_cont, new_prev_cont, new_cont, right_cont;
  wabi_cont_prompt res_prompt, new_prompt, prev_new_prompt, right_prompt;

  wabi_val ctrl, fst;

  ctrl = vm->ctrl;
  cont = cont0;

  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  fst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  new_cont = (wabi_cont) wabi_cont_copy(vm, cont);
  if(vm->ert) return;

  res_cont = new_cont;
  res_prompt = (wabi_cont_prompt) wabi_cont_done;
  new_prompt = (wabi_cont_prompt) wabi_cont_done;

  for(;;) {
    if(WABI_IS(wabi_tag_cont_prompt, cont) && !wabi_cont_next((wabi_cont) cont))
      break;
    prev_cont = cont;
    new_prev_cont = new_cont;
    cont = wabi_cont_next(cont);
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
  right_cont = wabi_cont_next((wabi_cont) vm->cont);
  right_prompt = (wabi_cont_prompt) vm->prmt;

  new_cont->next = (wabi_word) right_cont | WABI_TAG(new_cont);

  vm->ctrl = fst;
  vm->cont = (wabi_val) res_cont;
  if(new_prompt) {
    new_prompt->next_prompt = (wabi_word) right_prompt;
    vm->prmt = (wabi_val) res_prompt;
  }
}


int
wabi_cont_cmp(const wabi_cont a, const wabi_cont b)
{
  int cmp;
  switch(WABI_TAG(a)) {
  case wabi_tag_cont_eval:
    return wabi_cmp((wabi_val) ((wabi_cont_eval) a)->next, (wabi_val) ((wabi_cont_eval) b)->next);

  case wabi_tag_cont_apply:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->args, (wabi_val) ((wabi_cont_apply) b)->args);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_apply) a)->env, (wabi_val) ((wabi_cont_apply) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_apply) a)->next, (wabi_val) ((wabi_cont_apply) b)->next);

  case wabi_tag_cont_call:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->combiner, (wabi_val) ((wabi_cont_call) b)->combiner);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_call) a)->env, (wabi_val) ((wabi_cont_call) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_call) a)->next, (wabi_val) ((wabi_cont_call) b)->next);

  case wabi_tag_cont_sel:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->left, (wabi_val) ((wabi_cont_sel) b)->left);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->right, (wabi_val) ((wabi_cont_sel) b)->right);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_sel) a)->env, (wabi_val) ((wabi_cont_sel) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_sel) a)->next, (wabi_val) ((wabi_cont_sel) b)->next);

  case wabi_tag_cont_args:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->data, (wabi_val) ((wabi_cont_args) b)->data);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->done, (wabi_val) ((wabi_cont_args) b)->done);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_args) a)->env, (wabi_val) ((wabi_cont_args) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_args) a)->next, (wabi_val) ((wabi_cont_args) b)->next);

  case wabi_tag_cont_def:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->pattern, (wabi_val) ((wabi_cont_def) b)->pattern);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_def) a)->env, (wabi_val) ((wabi_cont_def) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_def) a)->next, (wabi_val) ((wabi_cont_def) b)->next);

  case wabi_tag_cont_prog:
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->expressions, (wabi_val) ((wabi_cont_prog) b)->expressions);
    if(cmp) return cmp;
    cmp = wabi_cmp((wabi_val) ((wabi_cont_prog) a)->env, (wabi_val) ((wabi_cont_prog) b)->env);
    if(cmp) return cmp;
    return wabi_cmp((wabi_val) ((wabi_cont_prog) a)->next, (wabi_val) ((wabi_cont_prog) b)->next);
  default:
    return -1000;
  }
}
