#define wabi_store_c

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_symbol.h"
#include "wabi_binary.h"
#include "wabi_list.h"
#include "wabi_map.h"
#include "wabi_env.h"
#include "wabi_cont.h"
#include "wabi_combiner.h"
#include "wabi_place.h"
#include "wabi_vector.h"


// todo: uncomment
// static inline void
void
wabi_copy_val_size(const wabi_vm vm, const wabi_val obj, const wabi_size size)
{
  wordcopy(vm->stor.heap, obj, size);
  vm->stor.heap += size;
}


static inline void
wabi_collect_binary_memcopy(char *dst, wabi_binary src) {
  wabi_word pivot;

  if(WABI_IS(wabi_tag_bin_leaf, src)) {
    memcpy(dst, (char *)((wabi_binary_leaf) src)->data_ptr, wabi_binary_length(src));
  } else {
    pivot = wabi_binary_length((wabi_binary) ((wabi_binary_node) src)->left);
    //todo use a loop?
    wabi_collect_binary_memcopy(dst, (wabi_binary) ((wabi_binary_node) src)->left);
    wabi_collect_binary_memcopy(dst + pivot, (wabi_binary) ((wabi_binary_node) src)->right);
  }
}


static inline void
wabi_collect_binary_copy_val(const wabi_vm vm, const wabi_binary src)
{
  wabi_size len, word_size;

  wabi_binary_leaf new_leaf;
  wabi_word *new_blob;
  len = wabi_binary_length((wabi_binary) src);
  word_size = wabi_binary_word_size(len);
  new_leaf = (wabi_binary_leaf) vm->stor.heap;
  vm->stor.heap += 2;
  new_blob = (wabi_word *) vm->stor.heap;
  vm->stor.heap += 1 + word_size;
  new_leaf->length = len;
  new_leaf->data_ptr = (wabi_word) (new_blob+1);
  *new_blob = 1 + word_size;
  WABI_SET_TAG(new_blob, wabi_tag_bin_blob);
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

  wabi_collect_binary_memcopy((char*)(new_blob + 1), (wabi_binary) src);
}


void
wabi_collect_env_copy_val(const wabi_vm vm,
                          const wabi_env env)
{
  wabi_size size;
  wabi_word *res;

  res = vm->stor.heap;
  size = env->numE * WABI_ENV_PAIR_SIZE;
  wordcopy(res, (wabi_word*) env, WABI_ENV_SIZE);
  wordcopy(res + WABI_ENV_SIZE, (wabi_word*) env->data, size);
  ((wabi_env)res)->data = (wabi_word) (res + WABI_ENV_SIZE);
  vm->stor.heap += WABI_ENV_SIZE + size;
}


wabi_val
wabi_copy_val(wabi_vm vm, wabi_val src)
{
  wabi_word* res;

  if(!src || (src >= vm->stor.new_space && src < vm->stor.limit))
    return src;

  res = vm->stor.heap;
  // printf("copy vm %p, val %p %s\n", vm, src, wabi_tag_to_string(src));
  switch(WABI_TAG(src)) {

  case wabi_tag_forward:
    return WABI_DEREF(src);

  case wabi_tag_fixnum:
  case wabi_tag_symbol:
  case wabi_tag_atom:
    *res = *src;
    vm->stor.heap++;
    break;

  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_collect_binary_copy_val(vm, (wabi_binary) src);
    break;

  case wabi_tag_pair:
    wabi_copy_val_size(vm, src, WABI_PAIR_SIZE);
    break;

  case wabi_tag_map_array:
  case wabi_tag_map_entry:
  case wabi_tag_map_hash:
    wabi_copy_val_size(vm, src, WABI_MAP_SIZE);
    break;

  case wabi_tag_env:
    wabi_collect_env_copy_val(vm, (wabi_env) src);
    break;

  case wabi_tag_cont_eval:
    wabi_copy_val_size(vm, src, WABI_CONT_EVAL_SIZE);
    break;

  case wabi_tag_cont_apply:
    wabi_copy_val_size(vm, src, WABI_CONT_APPLY_SIZE);
    break;

  case wabi_tag_cont_call:
    wabi_copy_val_size(vm, src, WABI_CONT_CALL_SIZE);
    break;

  case wabi_tag_cont_def:
    wabi_copy_val_size(vm, src, WABI_CONT_DEF_SIZE);
    break;

  case wabi_tag_cont_prog:
    wabi_copy_val_size(vm, src, WABI_CONT_PROG_SIZE);
    break;

  case wabi_tag_cont_args:
    wabi_copy_val_size(vm, src, WABI_CONT_ARGS_SIZE);
    break;

  case wabi_tag_cont_prompt:
    wabi_copy_val_size(vm, src, WABI_CONT_PROMPT_SIZE);
    break;

  case wabi_tag_cont_sel:
    wabi_copy_val_size(vm, src, WABI_CONT_SEL_SIZE);
    break;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wabi_copy_val_size(vm, src, WABI_COMBINER_BUILTIN_SIZE);
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
    wabi_copy_val_size(vm, src, WABI_COMBINER_DERIVED_SIZE);
    break;

  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    wabi_copy_val_size(vm,src, WABI_COMBINER_CONTINUATION_SIZE);
    break;

  case wabi_tag_place:
    wabi_copy_val_size(vm, src, WABI_PLACE_SIZE);
    break;

  case wabi_tag_vector_digit:
    wabi_copy_val_size(vm, src, WABI_VECTOR_DIGIT_SIZE + wabi_vector_digit_node_size((wabi_vector_digit) src));
    break;

  case wabi_tag_vector_deep:
    wabi_copy_val_size(vm, src, WABI_VECTOR_DEEP_SIZE);
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

  case wabi_tag_fixnum:
    vm->stor.scan++;
    break;

  case wabi_tag_symbol:
    wabi_symbol_collect_val(vm, (wabi_symbol) val);
    break;

  case wabi_tag_atom:
    wabi_atom_collect_val(vm, (wabi_atom) val);
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

  case wabi_tag_place:
    wabi_place_collect_val(vm, (wabi_place) val);
    break;

  case wabi_tag_vector_digit:
    wabi_vector_digit_collect_val(vm, (wabi_vector_digit) val);
    break;

  case wabi_tag_vector_deep:
    wabi_vector_deep_collect_val(vm, (wabi_vector_deep) val);
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
