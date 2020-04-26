#include "wabi_value.h"
#include "wabi_cont.h"
#include "wabi_cmp.h"
#include "wabi_vm.h"
#include "wabi_combiner.h"

#include "wabi_pr.h"
#include <stdio.h>

#define wabi_cont_c


static inline wabi_size
wabi_cont_size(wabi_cont cont)
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
wabi_cont_copy_val(wabi_store store, wabi_cont cont)
{
  wabi_size size = wabi_cont_size(cont);
  wordcopy(store->heap, (wabi_word*) cont, size);
  store->heap += size;
}


void
wabi_cont_collect_val(wabi_store store, wabi_cont cont)
{
  wabi_word tag, *next;

  next = (wabi_word*) wabi_cont_next(cont);
  if(next)
    cont->next = (wabi_word) wabi_store_copy_val(store, next);

  switch(tag = WABI_TAG(cont)) {
  case wabi_tag_cont_eval:
    store->scan += WABI_CONT_EVAL_SIZE;
    break;
  case wabi_tag_cont_prompt:
    if (((wabi_cont_prompt) cont)->tag) {
      ((wabi_cont_prompt) cont)->tag =
        (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_prompt) cont)->tag);
    }
    if(((wabi_cont_prompt) cont)->next_prompt) {
      ((wabi_cont_prompt) cont)->next_prompt =
        (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_prompt) cont)->next_prompt);
    }
    store->scan += WABI_CONT_PROMPT_SIZE;
    break;
  case wabi_tag_cont_apply:
    ((wabi_cont_apply) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_apply) cont)->env);
    ((wabi_cont_apply) cont)->args =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_apply) cont)->args);
    store->scan += WABI_CONT_APPLY_SIZE;
    break;
  case wabi_tag_cont_call:
    ((wabi_cont_call) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_call) cont)->env);
    ((wabi_cont_call) cont)->combiner =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_call) cont)->combiner);
    store->scan += WABI_CONT_CALL_SIZE;
    break;
  case wabi_tag_cont_sel:
    ((wabi_cont_sel) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_sel) cont)->env);
    ((wabi_cont_sel) cont)->left =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_sel) cont)->left);
    ((wabi_cont_sel) cont)->right =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_sel) cont)->right);
    store->scan += WABI_CONT_SEL_SIZE;
    break;
  case wabi_tag_cont_args:
    ((wabi_cont_args) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_args) cont)->env);
    ((wabi_cont_args) cont)->data =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_args) cont)->data);
    ((wabi_cont_args) cont)->done =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_args) cont)->done);
    store->scan += WABI_CONT_ARGS_SIZE;
    break;
  case wabi_tag_cont_def:
    ((wabi_cont_def) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_def) cont)->env);
    ((wabi_cont_def) cont)->pattern =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_def) cont)->pattern);
    store->scan += WABI_CONT_DEF_SIZE;
    break;
  case wabi_tag_cont_prog:
    ((wabi_cont_prog) cont)->env =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_prog) cont)->env);
    ((wabi_cont_prog) cont)->expressions =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_prog) cont)->expressions);
    store->scan += WABI_CONT_PROG_SIZE;
    break;
  default:
    break;
  }
  WABI_SET_TAG(cont, tag);
}


/**
 * hashing
 */
void
wabi_cont_hash(wabi_hash_state state, wabi_cont cont)
{
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
wabi_cont_concat_cont(wabi_vm vm, wabi_cont cont)
{
  // TBD
  vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
  return;
}

wabi_error_type
wabi_cont_cmp(wabi_cont a, wabi_cont b)
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
