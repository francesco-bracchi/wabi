#define wabi_cont_c

#include "wabi_cont.h"
#include "wabi_value.h"
#include "wabi_store.h"

wabi_cont
wabi_cont_pop(wabi_store store)
{
  wabi_cont cont;

  cont = (wabi_cont) store->stack;
  store->stack = (wabi_val) WABI_WORD_VAL(cont->prev);
  return cont;
}

wabi_cont
wabi_cont_eval_push(wabi_store store, wabi_val env)
{
  wabi_word *prev = store->stack;
  wabi_cont_eval cont = (wabi_cont_eval) wabi_store_stack_alloc(store, WABI_CONT_EVAL_SIZE);
  cont->cont.prev = (wabi_word) prev;
  cont->cont.env = (wabi_word) env;
  WABI_SET_TAG(cont, wabi_tag_cont_eval);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_apply_push(wabi_store store, wabi_val env, wabi_val arguments)
{
  wabi_word *prev = store->stack;
  wabi_cont_apply cont = (wabi_cont_apply) wabi_store_stack_alloc(store, WABI_CONT_APPLY_SIZE);
  cont->cont.prev = (wabi_word) prev;
  cont->cont.env = (wabi_word) env;
  cont->arguments = (wabi_word) arguments;
  WABI_SET_TAG(cont, wabi_tag_cont_apply);
  return (wabi_cont) cont;
}
