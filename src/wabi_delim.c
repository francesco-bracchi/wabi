#define wabi_delim_c

#include "wabi_delim.h"
#include "wabi_builtin.h"
#include "wabi_cmp.h"
#include "wabi_constant.h"
#include "wabi_cont.h"
#include "wabi_vm.h"

static void wabi_cont_prompt_bt(const wabi_vm vm) {
  wabi_env env;
  wabi_val fst, ctrl, tag;
  wabi_cont cont, prmt;

  ctrl = vm->ctrl;

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  tag = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  fst = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);
  env = (wabi_env)((wabi_cont_call)vm->cont)->env;

  cont = wabi_cont_next((wabi_cont)vm->cont);
  prmt = cont =
      wabi_cont_push_prompt(vm, tag, (wabi_cont_prompt)vm->prmt, cont);
  if (vm->ert)
    return;

  if (wabi_is_pair(ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if (vm->ert)
      return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if (vm->ert)
    return;

  vm->ctrl = fst;
  vm->cont = (wabi_val)cont;
  vm->prmt = (wabi_val)prmt;
}

static void wabi_cont_control_bt(const wabi_vm vm) {
  wabi_env env;
  wabi_val kname, ctrl, tag, fst;
  wabi_cont_prompt prompt;
  wabi_cont cont;
  wabi_combiner kval;
  wabi_error_type err;

  ctrl = vm->ctrl;
  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  tag = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  kname = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  if (!wabi_is_symbol(kname) && !wabi_is_ignore(kname)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  if (!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }

  fst = wabi_car((wabi_pair)ctrl);
  ctrl = wabi_cdr((wabi_pair)ctrl);

  env = (wabi_env)((wabi_cont_call)vm->cont)->env;
  env = wabi_env_extend(vm, env);
  if (vm->ert)
    return;

  cont = wabi_cont_next((wabi_cont)vm->cont);
  prompt = (wabi_cont_prompt)vm->prmt;

  kval = wabi_combiner_continuation_new(vm, cont);
  if (vm->ert)
    return;

  for (;;) {
    if (!prompt) {
      vm->ert = wabi_error_no_prompt;
      return;
    }

    if (wabi_eq(tag, (wabi_val)wabi_cont_prompt_tag(prompt)))
      break;

    prompt = wabi_cont_prompt_next_prompt(prompt);
  }
  wabi_env_set(vm, env, kname, (wabi_val)kval);
  if (vm->ert)
    return;

  cont = wabi_cont_next((wabi_cont)prompt);

  if (wabi_is_pair(ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if (vm->ert)
      return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if (vm->ert)
    return;

  vm->ctrl = fst;
  vm->env = (wabi_val)env;
  vm->cont = (wabi_val)cont;
  vm->prmt = (wabi_val)wabi_cont_prompt_next_prompt(prompt);

  prompt->tag = (wabi_word)vm->nil;
  prompt->next_prompt = (wabi_word)wabi_cont_done;
  prompt->next = (wabi_word)wabi_cont_done;
  WABI_SET_TAG(prompt, wabi_tag_cont_prompt);
}

void wabi_delim_builtins(const wabi_vm vm, const wabi_env env) {
  WABI_DEFX(vm, env, "prompt", "prompt", wabi_cont_prompt_bt);
  if (vm->ert)
    return;
  WABI_DEFX(vm, env, "control", "control", wabi_cont_control_bt);
}
