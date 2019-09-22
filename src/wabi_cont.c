#define wabi_cont_c

#include "wabi_vm.h"
#include "wabi_cont.h"
#include "wabi_value.h"
#include "wabi_env.h"

wabi_cont
wabi_cont_eval_new(wabi_vm vm, wabi_env env, wabi_cont prev)
{
  wabi_cont_eval cont;
  cont = (wabi_cont_eval) wabi_vm_alloc(vm, WABI_CONT_EVAL_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  WABI_SET_TAG(cont, wabi_tag_cont_eval);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_apply_new(wabi_vm vm, wabi_env env, wabi_val args, wabi_cont prev)
{
  wabi_cont_apply cont;
  cont = (wabi_cont_apply) wabi_vm_alloc(vm, WABI_CONT_APPLY_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  cont->args = (wabi_word) args;
  WABI_SET_TAG(cont, wabi_tag_cont_apply);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_call_new(wabi_vm vm, wabi_env env, wabi_val combiner, wabi_cont prev)
{
  wabi_cont_call cont;
  cont = (wabi_cont_call) wabi_vm_alloc(vm, WABI_CONT_CALL_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  cont->combiner = (wabi_word) combiner;
  WABI_SET_TAG(cont, wabi_tag_cont_call);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_sel_new(wabi_vm vm, wabi_env env, wabi_val left, wabi_val right, wabi_cont prev)
{
  wabi_cont_sel cont;
  cont = (wabi_cont_sel) wabi_vm_alloc(vm, WABI_CONT_SEL_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  cont->left = (wabi_word) left;
  cont->right = (wabi_word) right;
  WABI_SET_TAG(cont, wabi_tag_cont_sel);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_eval_more_new(wabi_vm vm, wabi_env env, wabi_val data, wabi_val done, wabi_cont prev)
{
  wabi_cont_eval_more cont;
  cont = (wabi_cont_eval_more) wabi_vm_alloc(vm, WABI_CONT_EVAL_MORE_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  cont->data = (wabi_word) data;
  cont->done = (wabi_word) done;
  WABI_SET_TAG(cont, wabi_tag_cont_eval_more);
  return (wabi_cont) cont;
}

wabi_cont
wabi_cont_def_new(wabi_vm vm, wabi_env env, wabi_val pattern, wabi_cont prev)
{
  wabi_cont_def cont;
  cont = (wabi_cont_def) wabi_vm_alloc(vm, WABI_CONT_DEF_SIZE);
  cont->prev = (wabi_word) prev;
  cont->env = (wabi_word) env;
  cont->pattern = (wabi_word) pattern;
  WABI_SET_TAG(cont, wabi_tag_cont_def);
  return (wabi_cont) cont;
}
