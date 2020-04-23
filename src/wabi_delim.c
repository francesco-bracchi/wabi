#define wabi_delim_c

#include "wabi_vm.h"
#include "wabi_cont.h"
#include "wabi_delim.h"
#include "wabi_builtin.h"
#include "wabi_cmp.h"

wabi_error_type
wabi_cont_prompt_bt(wabi_vm vm)
{
  wabi_env env;
  wabi_val fst, ctrl, tag, prmt;
  wabi_cont cont;

  ctrl = vm->control;

  if(!WABI_IS(wabi_tag_pair, ctrl)) return wabi_error_type_mismatch;

  tag = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!WABI_IS(wabi_tag_pair, ctrl)) return wabi_error_type_mismatch;

  fst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;

  cont = wabi_cont_next((wabi_cont) vm->continuation);
  cont = wabi_cont_push_prompt(vm, tag, cont);

  if(! cont) return wabi_error_nomem;

  prmt = (wabi_val) wabi_cons(vm, (wabi_val) cont, (wabi_val) vm->prompt);
  if(! prmt) return wabi_error_nomem;

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(! cont) return wabi_error_nomem;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if(! cont) return wabi_error_nomem;

  vm->control = fst;
  vm->continuation = (wabi_val) cont;
  vm->prompt = prmt;
  return wabi_error_none;
}


wabi_error_type
wabi_cont_control_bt(wabi_vm vm)
{
  wabi_env env;
  wabi_val kname, ctrl, tag, fst;
  wabi_pair pstack;
  wabi_cont cont;
  wabi_combiner kval;
  wabi_error_type err;

  ctrl = vm->control;
  if(! WABI_IS(wabi_tag_pair, ctrl)) return wabi_error_type_mismatch;

  tag = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! WABI_IS(wabi_tag_pair, ctrl)) return wabi_error_type_mismatch;

  kname = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! WABI_IS(wabi_tag_symbol, kname) && (*kname != wabi_val_ignore))
    return wabi_error_type_mismatch;

  if(! WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_type_mismatch;

  fst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
  env = wabi_env_extend(vm, env);
  if(!env) return wabi_error_nomem;

  cont = wabi_cont_next((wabi_cont) vm->continuation);
  pstack = (wabi_pair) vm->prompt;
  kval = wabi_combiner_continuation_new(vm, tag, cont, pstack);

  if(!kval) return wabi_error_nomem;

  for(;;) {
    if(!WABI_IS(wabi_tag_pair, pstack)) {
      return wabi_error_no_prompt;
    }
    cont = (wabi_cont) wabi_car(pstack);
    if(wabi_eq(tag, (wabi_val) ((wabi_cont_prompt) cont)->tag)) {
      cont = wabi_cont_next(cont);
      break;
    }
    pstack = (wabi_pair) wabi_cdr(pstack);
  }

  err = wabi_env_set(vm, env, kname, (wabi_val) kval);
  if(err) return err;

  if(*ctrl == wabi_val_nil) {
    cont = wabi_cont_push_eval(vm, cont);
    if(! cont) return wabi_error_nomem;
    vm->control = fst;
    vm->env = (wabi_val) env;
    vm->continuation = (wabi_val) cont;
    return wabi_error_none;
  }

  if(WABI_IS(wabi_tag_pair, ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(! cont) return wabi_error_nomem;
    cont = wabi_cont_push_eval(vm, cont);
    if(! cont) return wabi_error_nomem;
    vm->control = fst;
    vm->env = (wabi_val) env;
    vm->continuation = (wabi_val) cont;
    return wabi_error_none;
  }
  return wabi_error_other;
}

wabi_error_type
wabi_delim_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFX(vm, env, "prompt", "prompt", wabi_cont_prompt_bt);
  if(res) return res;
  res = WABI_DEFX(vm, env, "control", "control", wabi_cont_control_bt);
  return res;
}
