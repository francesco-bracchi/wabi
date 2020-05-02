#define wabi_store_c

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_symbol.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_map.h"
#include "wabi_env.h"
#include "wabi_cont.h"
#include "wabi_combiner.h"

wabi_val
wabi_copy_val(wabi_vm vm, wabi_val src)
{
  wabi_word* res;
  wabi_size size;

  if(! src) return src;

  res = vm->stor.heap;
  // printf("copy vm %p, val %p %s\n", vm, src, wabi_tag_to_string(src));
  switch(WABI_TAG(src)) {

  case wabi_tag_forward:
    return WABI_DEREF(src);

  case wabi_tag_constant:
  case wabi_tag_fixnum:
    *res = *src;
    vm->stor.heap++;
    break;

  case wabi_tag_symbol:
    wabi_symbol_copy_val(vm, (wabi_symbol) src);
    break;

  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_binary_copy_val(vm, (wabi_binary) src);
    break;

  case wabi_tag_pair:
    wabi_pair_copy_val(vm, (wabi_pair) src);
    break;

  case wabi_tag_map_array:
    wabi_map_array_copy_val(vm, (wabi_map_array) src);
    break;

  case wabi_tag_map_entry:
    wabi_map_entry_copy_val(vm, (wabi_map_entry) src);
    break;

  case wabi_tag_map_hash:
    wabi_map_hash_copy_val(vm, (wabi_map_hash) src);
    break;

  case wabi_tag_env:
    wabi_env_copy_val(vm, (wabi_env) src);
    break;

  case wabi_tag_cont_eval:
    wabi_cont_eval_copy_val(vm, (wabi_cont_eval) src);
    break;

  case wabi_tag_cont_apply:
    wabi_cont_apply_copy_val(vm, (wabi_cont_apply) src);
    break;

  case wabi_tag_cont_call:
    wabi_cont_call_copy_val(vm, (wabi_cont_call) src);
    break;

  case wabi_tag_cont_def:
    wabi_cont_def_copy_val(vm, (wabi_cont_def) src);
    break;

  case wabi_tag_cont_prog:
    wabi_cont_prog_copy_val(vm, (wabi_cont_prog) src);
    break;

  case wabi_tag_cont_args:
    wabi_cont_args_copy_val(vm, (wabi_cont_args) src);
    break;

  case wabi_tag_cont_prompt:
    wabi_cont_prompt_copy_val(vm, (wabi_cont_prompt) src);
    break;

  case wabi_tag_cont_sel:
    wabi_cont_sel_copy_val(vm, (wabi_cont_sel) src);
    break;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wabi_combiner_builtin_copy_val(vm, (wabi_combiner_builtin) src);
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
    wabi_combiner_derived_copy_val(vm, (wabi_combiner_derived) src);
    break;

  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    wabi_combiner_continuation_copy_val(vm, (wabi_combiner_continuation) src);
    break;
  }

  *src = (wabi_word) res;
  WABI_SET_TAG(src, wabi_tag_forward);
  return res;
}


static inline void
wabi_collect_val(wabi_vm vm, wabi_val val)
{
  // printf("collect vm %p, val %p %s\n", vm, val, wabi_tag_to_string(val));
  switch(WABI_TAG(val)) {

  case wabi_tag_bin_blob:
    vm->stor.scan += WABI_WORD_VAL(*val);
    break;

  case wabi_tag_constant:
  case wabi_tag_fixnum:
    vm->stor.scan++;
    break;

  case wabi_tag_symbol:
    wabi_symbol_collect_val(vm, (wabi_symbol) val);
    break;

  case wabi_tag_bin_leaf:
    wabi_binary_collect_val(vm, (wabi_binary) val);
    break;

  case wabi_tag_pair:
    wabi_pair_collect_val(vm, (wabi_pair) val);
    break;

  case wabi_tag_map_entry:
    wabi_map_entry_collect_val(vm, (wabi_map_entry) val);
    break;
  case wabi_tag_map_array:
    wabi_map_array_collect_val(vm, (wabi_map_array) val);
    break;
  case wabi_tag_map_hash:
    wabi_map_hash_collect_val(vm, (wabi_map_hash) val);
    break;

  case wabi_tag_oper:
  case wabi_tag_app:
    wabi_combiner_derived_collect_val(vm, (wabi_combiner_derived) val);
    break;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wabi_combiner_builtin_collect_val(vm, (wabi_combiner_builtin) val);
    break;

  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    wabi_combiner_continuation_collect_val(vm, (wabi_combiner_continuation) val);
    break;

  case wabi_tag_env:
    wabi_env_collect_val(vm, (wabi_env) val);
    break;

  case wabi_tag_cont_eval:
    wabi_cont_eval_collect_val(vm, (wabi_cont_eval) val);
    break;

  case wabi_tag_cont_prompt:
    wabi_cont_prompt_collect_val(vm, (wabi_cont_prompt) val);
    break;

  case wabi_tag_cont_apply:
    wabi_cont_apply_collect_val(vm, (wabi_cont_apply) val);
    break;

  case wabi_tag_cont_call:
    wabi_cont_call_collect_val(vm, (wabi_cont_call) val);
    break;

  case wabi_tag_cont_def:
    wabi_cont_def_collect_val(vm, (wabi_cont_def) val);
    break;

  case wabi_tag_cont_prog:
    wabi_cont_prog_collect_val(vm, (wabi_cont_prog) val);
    break;

  case wabi_tag_cont_args:
    wabi_cont_args_collect_val(vm, (wabi_cont_args) val);
    break;

  case wabi_tag_cont_sel:
    wabi_cont_sel_collect_val(vm, (wabi_cont_sel) val);
    break;

  case wabi_tag_forward:
    *val = WABI_WORD_VAL(*val);
    vm->stor.scan++;
    break;
  }
}


/**
 * Collect should be interruptible, i.e. the vm can be interrupted even
 * during the collection part, what's is missing is a boolean value
 * on the vm, that let the reducer to finish the GC phase once resumed
 */
void
wabi_collect(wabi_vm vm)
{
  do {
    wabi_collect_val(vm, vm->stor.scan);
    // vm->fuel--;
  } while(vm->stor.scan < vm->stor.heap /* && vm->fuel */);
}
