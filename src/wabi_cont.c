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

void
wabi_cont_eval_push(wabi_store store, wabi_val env)
{
  wabi_cont_eval cont;
  cont = (wabi_cont_eval) wabi_store_stack_alloc(store, WABI_CONT_EVAL_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  WABI_SET_TAG(cont, wabi_tag_cont_eval);
  store->stack = (wabi_word *) cont;
}

void
wabi_cont_apply_push(wabi_store store, wabi_val env, wabi_val args)
{
  wabi_cont_apply cont;
  cont = (wabi_cont_apply) wabi_store_stack_alloc(store, WABI_CONT_APPLY_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  cont->args = (wabi_word) args;
  WABI_SET_TAG(cont, wabi_tag_cont_apply);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_call_push(wabi_store store, wabi_val env, wabi_val combiner)
{
  wabi_cont_call cont;
  cont = (wabi_cont_call) wabi_store_stack_alloc(store, WABI_CONT_CALL_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  cont->combiner = (wabi_word) combiner;
  WABI_SET_TAG(cont, wabi_tag_cont_call);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_sel_push(wabi_store store, wabi_val env, wabi_val left, wabi_val right)
{
  wabi_cont_sel cont;
  cont = (wabi_cont_sel) wabi_store_stack_alloc(store, WABI_CONT_SEL_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  cont->left = (wabi_word) left;
  cont->right = (wabi_word) right;
  WABI_SET_TAG(cont, wabi_tag_cont_sel);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_eval_all_push(wabi_store store, wabi_val env)
{
  wabi_cont_eval_all cont;
  cont = (wabi_cont_eval_all) wabi_store_stack_alloc(store, WABI_CONT_EVAL_ALL_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  WABI_SET_TAG(cont, wabi_tag_cont_eval_all);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_eval_more_push(wabi_store store, wabi_val env, wabi_val data, wabi_val done)
{
  wabi_cont_eval_more cont;
  cont = (wabi_cont_eval_more) wabi_store_stack_alloc(store, WABI_CONT_EVAL_MORE_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  cont->data = (wabi_word) data;
  cont->done = (wabi_word) done;
  WABI_SET_TAG(cont, wabi_tag_cont_eval_more);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_eval_rev_push(wabi_store store, wabi_val data)
{
  wabi_cont_eval_rev cont;
  cont = (wabi_cont_eval_rev) wabi_store_stack_alloc(store, WABI_CONT_EVAL_REV_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->data = (wabi_word) data;
  WABI_SET_TAG(cont, wabi_tag_cont_eval_rev);
  store->stack = (wabi_word*) cont;
}

void
wabi_cont_prog_push(wabi_store store, wabi_val env, wabi_val ctrls)
{
  wabi_cont_prog cont;
  cont = (wabi_cont_prog) wabi_store_stack_alloc(store, WABI_CONT_PROG_SIZE);
  cont->prev = (wabi_word) store->stack;
  cont->env = (wabi_word) env;
  cont->ctrls = (wabi_word) ctrls;
  WABI_SET_TAG(cont, wabi_tag_cont_prog);
  store->stack = (wabi_word*) cont;
}
