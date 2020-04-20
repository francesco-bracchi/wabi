#include "wabi_value.h"
#include "wabi_cont.h"
#include "wabi_cmp.h"
#include "wabi_vm.h"
#include "wabi_combiner.h"

#include "wabi_pr.h"
#include <stdio.h>

#define wabi_cont_c

/**
 * do not eagerly concatenate, do it lazily, without consuming more memory
 */

static inline wabi_cont
wabi_cont_concat_in(wabi_vm vm, wabi_val tag0, wabi_val lcont, wabi_cont k)
{
  wabi_word wtag;
  wabi_cont res, nxt;

  switch(wtag = WABI_TAG(lcont)) {
  case wabi_tag_cont_prompt:
    if(wabi_eq(tag0, (wabi_val) ((wabi_cont_prompt) lcont)->tag)) {
      return k;
    }
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_PROMPT_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_prompt_t));
    break;

  case wabi_tag_cont_eval:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_EVAL_SIZE);
    if(! res) return NULL;
    memcpy(res, lcont, sizeof(wabi_cont_eval_t));
    break;
  case wabi_tag_cont_apply:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_APPLY_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_apply_t));
    break;
  case wabi_tag_cont_call:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_CALL_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_call_t));
    break;
  case wabi_tag_cont_sel:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_SEL_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_sel_t));
    break;
  case wabi_tag_cont_args:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_ARGS_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_args_t));
    break;
  case wabi_tag_cont_def:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_DEF_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_def_t));
    break;
  case wabi_tag_cont_prog:
    res = (wabi_cont) wabi_vm_alloc(vm, WABI_CONT_PROG_SIZE);
    memcpy(res, lcont, sizeof(wabi_cont_prog_t));
    break;
  default:
    return NULL;
  }
  nxt = wabi_cont_concat_in(vm, tag0, (wabi_val) WABI_WORD_VAL(((wabi_cont) lcont)->next), k);
  if(! nxt) return NULL;
  res->next = (wabi_word) nxt;
  WABI_SET_TAG(res, wtag);
  return res;
}

wabi_cont
wabi_cont_concat(wabi_vm vm, wabi_val l, wabi_cont k)
{
  wabi_val tag = (wabi_val) WABI_WORD_VAL(((wabi_combiner_continuation)l)->tag);
  wabi_val lcont = (wabi_val) ((wabi_combiner_continuation)l)->cont;
  return wabi_cont_concat_in(vm, tag, lcont, k);
}


void
wabi_cont_copy_val(wabi_store store, wabi_cont cont)
{
  switch(WABI_TAG(cont)) {
  case wabi_tag_cont_eval:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_EVAL_SIZE);
    store->heap += WABI_CONT_EVAL_SIZE;
    break;
  case wabi_tag_cont_prompt:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_PROMPT_SIZE);
    store->heap += WABI_CONT_PROMPT_SIZE;
    break;
  case wabi_tag_cont_apply:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_APPLY_SIZE);
    store->heap += WABI_CONT_APPLY_SIZE;
    break;

  case wabi_tag_cont_call:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_CALL_SIZE);
    store->heap += WABI_CONT_CALL_SIZE;
    break;

  case wabi_tag_cont_sel:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_SEL_SIZE);
    store->heap += WABI_CONT_SEL_SIZE;
    break;

  case wabi_tag_cont_args:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_ARGS_SIZE);
    store->heap += WABI_CONT_ARGS_SIZE;
    break;

  case wabi_tag_cont_def:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_DEF_SIZE);
    store->heap += WABI_CONT_DEF_SIZE;
    break;

  case wabi_tag_cont_prog:
    wordcopy(store->heap, (wabi_word*) cont, WABI_CONT_PROG_SIZE);
    store->heap += WABI_CONT_PROG_SIZE;
    break;
  default:
    break;
  }
}


void
wabi_cont_collect_val(wabi_store store, wabi_cont cont)
{
  wabi_word tag, *next;

  switch(tag = WABI_TAG(cont)) {
  case wabi_tag_cont_eval:
    store->scan += WABI_CONT_EVAL_SIZE;
    break;
  case wabi_tag_cont_prompt:
    ((wabi_cont_prompt) cont)->tag =
      (wabi_word) wabi_store_copy_val(store, (wabi_word*) ((wabi_cont_prompt) cont)->tag);
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
  next = wabi_cont_next(cont);
  if(next)
    cont->next = (wabi_word) wabi_store_copy_val(store, next);

  WABI_SET_TAG(cont, tag);
}
