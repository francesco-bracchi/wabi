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

  ctrl = vm->control;
  if(WABI_IS(wabi_tag_pair, ctrl)) {
    tag = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_symbol, tag) && WABI_IS(wabi_tag_pair, ctrl)) {
      fst = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      env = (wabi_env) ((wabi_cont_call) vm->continuation)->env;
      if(*ctrl == wabi_val_nil) {
        if(wabi_vm_has_rooms(vm, WABI_CONT_PROMPT_SIZE)) {
          wabi_cont_pop(vm);
          wabi_cont_push_prompt(vm, env, tag);
          vm->control = fst;
          return wabi_error_none;
        }
        return wabi_error_nomem;
      }
      if(WABI_IS(wabi_tag_pair, ctrl)) {
        if(wabi_vm_has_rooms(vm, WABI_CONT_PROMPT_SIZE + WABI_CONT_PROG_SIZE)) {
          wabi_cont_pop(vm);
          wabi_cont_push_prog(vm, env, ctrl);
          wabi_cont_push_prompt(vm, env, tag);
          vm->control = fst;
          return wabi_error_none;
        }
        return wabi_error_nomem;
      }
    }
    return wabi_error_type_mismatch;
  }
}

wabi_error_type
wabi_delim_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFX(vm, env, "prompt", "wabi:prompt", wabi_cont_prompt_bt);
  if(res) return res;
  return res;
}
