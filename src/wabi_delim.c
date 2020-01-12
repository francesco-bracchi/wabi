#define wabi_delim_c

#include "wabi_vm.h"
#include "wabi_cont.h"
#include "wabi_delim.h"
#include "wabi_builtin.h"


wabi_error_type
wabi_cont_prompt_bt(wabi_vm vm)
{
  wabi_env env;
  wabi_val fst, ctrl, tag;
  wabi_cont cont;

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    tag = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_symbol, tag) && WABI_IS(wabi_tag_pair, ctrl)) {
      fst = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
      if(*ctrl == wabi_val_nil) {
        cont = wabi_cont_next((wabi_cont) vm->continuation);
        cont = wabi_cont_push_prompt(vm, env, tag, cont);
        if(cont) {
          vm->control = fst;
          vm->continuation = (wabi_val) cont;
          return wabi_error_none;
        }
        return wabi_error_nomem;
      }
      if(WABI_IS(wabi_tag_pair, ctrl)) {
        cont = wabi_cont_next((wabi_cont) vm->continuation);
        cont = wabi_cont_push_prog(vm, env, ctrl, cont);
        if(cont) {
          cont = wabi_cont_push_prompt(vm, env, tag, cont);
          if(cont) {
            vm->control = fst;
            vm->continuation = (wabi_val) cont;
          }
        }
        return wabi_error_nomem;
      }
    }
  }
  return wabi_error_type_mismatch;
}

wabi_error_type
wabi_delim_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFX(vm, env, "prompt", "wabi:prompt", wabi_cont_prompt_bt);
  return res;
}
