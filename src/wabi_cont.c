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
    /* printf("TESTING PROMPT "); */
    /* wabi_prn((wabi_val) ((wabi_cont_prompt) lcont)->tag); */
    /* printf("against "); */
    /* wabi_prn(tag0); */
    /* printf("result %i\n", wabi_eq(tag0, (wabi_val) ((wabi_cont_prompt) lcont)->tag)); */
    if(wabi_eq(tag0, (wabi_val) ((wabi_cont_prompt) lcont)->tag)) {
      /* printf("---------------------------------------\n"); */
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
