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


static inline void
wabi_copy_val_size(const wabi_vm vm, const wabi_val obj, const wabi_size size)
{
  wordcopy(vm->stor.heap, obj, size);
  vm->stor.heap += size;
}

static inline void
wabi_collect_binary_memcopy(char *dst, wabi_binary src) {
  wabi_word pivot;

  while (WABI_IS(wabi_tag_bin_node, src)) {
    pivot = wabi_binary_length((wabi_binary) ((wabi_binary_node) src)->left);
    //todo use a loop?
    wabi_collect_binary_memcopy(dst, (wabi_binary) ((wabi_binary_node) src)->left);
    dst += pivot;
    src = (wabi_binary) ((wabi_binary_node) src)->right;
  }
  if(WABI_IS(wabi_tag_bin_leaf, src)) {
    memcpy(dst, (char *)((wabi_binary_leaf) src)->data_ptr, wabi_binary_length(src));
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


static inline void
wabi_collect_env_copy_val(const wabi_vm vm,
                          const wabi_env env)
{
  wabi_size size;
  wabi_word *res;

  res = vm->stor.heap;
  wordcopy(res, (wabi_word*) env, WABI_ENV_SIZE);
  size = env->maxE * WABI_ENV_PAIR_SIZE;
  vm->stor.heap += WABI_ENV_SIZE + size;
}

static inline void
wabi_collect_sym_copy_val(const wabi_vm vm, const wabi_symbol src)
{
  wabi_val old_sym = (wabi_val) src;
  wabi_val old_bin = WABI_DEREF(old_sym);

  wabi_symbol new_sym = (wabi_symbol) vm->stor.heap;
  vm->stor.heap++;
  wabi_binary new_bin = (wabi_binary) vm->stor.heap;
  wabi_collect_binary_copy_val(vm, (wabi_binary) old_bin);

  *new_sym = (wabi_word) new_bin;
  WABI_SET_TAG(new_sym, wabi_tag_symbol);

  *old_bin = (wabi_word) new_bin;
  WABI_SET_TAG(old_bin, wabi_tag_forward);
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
  case wabi_tag_atom:
    *res = *src;
    vm->stor.heap++;
    break;

  case wabi_tag_symbol:
    wabi_collect_sym_copy_val(vm, (wabi_symbol) src);
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

  case wabi_tag_ct:
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

// reverse order, because the first word contains references to a kind of tail
// like pairs, the tail part is the first, byte, the same for continuations.
static inline void
wabi_collect_val_size(const wabi_vm vm, const wabi_val obj, const wabi_size size)
{
  wabi_size cnt;
  wabi_word* pos;
  wabi_word tag;

  tag = WABI_WORD_TAG(*obj);
  cnt = 0;
  pos = obj + (size - 1);
  while(cnt < size) {
    *pos = (wabi_word) wabi_copy_val(vm, (wabi_word*) WABI_WORD_VAL(*pos));
    cnt++;
    pos--;
  }
  WABI_SET_TAG(obj, tag);
  vm->stor.scan+= size;
}

static inline void
wabi_collect_symbol(const wabi_vm vm, const wabi_val sym)
{
  wabi_val binref;
  wabi_map tbl;

  binref = wabi_symbol_to_binary(sym);
  binref = wabi_copy_val(vm, binref);
  *sym = (wabi_word) binref;
  WABI_SET_TAG(sym, wabi_tag_symbol);
  vm->stor.scan+= WABI_SYMBOL_SIZE;

  if(wabi_map_get((wabi_map) vm->stbl, binref))
    return;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->stbl, binref, sym);
  if(vm->ert) return;
  vm->stbl = (wabi_val) tbl;
}

static inline void
wabi_collect_atom(const wabi_vm vm, const wabi_val atm)
{
  wabi_val binref;
  wabi_map tbl;

  binref = wabi_atom_to_binary(atm);
  binref = wabi_copy_val(vm, binref);
  *atm = (wabi_word) binref;
  WABI_SET_TAG(atm, wabi_tag_atom);
  vm->stor.scan+= WABI_ATOM_SIZE;

  if(wabi_map_get((wabi_map) vm->atbl, binref))
    return;

  tbl = wabi_map_assoc(vm, (wabi_map) vm->atbl, binref, atm);
  if(vm->ert) return;
  vm->atbl = (wabi_val) tbl;
}

static inline void
wabi_collect_map_array(wabi_vm vm, wabi_map_array array)
{
  wabi_word size;
  size = array->size;

  wordcopy(vm->stor.heap, (wabi_word*) WABI_WORD_VAL(array->table), wabi_sizeof(wabi_map_entry_t) * size);
  array->table = (wabi_word) vm->stor.heap;
  vm->stor.heap += wabi_sizeof(wabi_map_entry_t) * size;
  WABI_SET_TAG(array, wabi_tag_map_array);
  vm->stor.scan += wabi_sizeof(wabi_map_array_t);
}


static inline void
wabi_collect_map_hash(wabi_vm vm, wabi_map_hash map)
{
  wabi_word size;
  size = WABI_MAP_BITMAP_COUNT(map->bitmap);

  wordcopy(vm->stor.heap, (wabi_word*) WABI_WORD_VAL(map->table), wabi_sizeof(wabi_map_entry_t) * size);
  map->table = (wabi_word) vm->stor.heap;
  vm->stor.heap += wabi_sizeof(wabi_map_entry_t) * size;
  WABI_SET_TAG(map, wabi_tag_map_hash);
  vm->stor.scan += wabi_sizeof(wabi_map_hash_t);
}

static inline void
wabi_collect_combiner_bt(const wabi_vm vm, const wabi_combiner_builtin c)
{
  vm->stor.scan += WABI_COMBINER_BUILTIN_SIZE;
}

static inline void
wabi_collect_env(wabi_vm vm, wabi_env env)
{
  wabi_env_pair old_data, p0;
  wabi_env_pair_t p;
  wabi_size size, j;


  old_data = (wabi_env_pair) env->data;
  size = env->maxE;
  env->data = (wabi_word)((wabi_val) env + WABI_ENV_SIZE);
  wabi_env_reset(env);

  for (j = 0; j < size; j++) {
    p0 = old_data + j;
    if (p0->key) {
      p.key = (wabi_word) wabi_copy_val(vm, (wabi_val) p0->key);
      p.val = (wabi_word) wabi_copy_val(vm, (wabi_val) p0->val);
      wabi_env_set_raw(env, &p);
    }
  }

  if(WABI_WORD_VAL(env->prev)) {
    env->prev = (wabi_word) wabi_copy_val(vm, (wabi_val) WABI_WORD_VAL(env->prev));
  }
  WABI_SET_TAG(env, wabi_tag_env);
  vm->stor.scan += WABI_ENV_SIZE + size * WABI_ENV_PAIR_SIZE;
}

static inline void
wabi_collect_prompt(const wabi_vm vm, const wabi_cont_prompt cont)
{
  cont->tag = (wabi_word) wabi_copy_val(vm, (wabi_val) WABI_WORD_VAL(cont->tag));
  if(cont->next != (wabi_word) wabi_cont_done) {
    cont->next = (wabi_word) wabi_copy_val(vm, (wabi_val) WABI_WORD_VAL(cont->next));
  }
  if(cont->next_prompt != (wabi_word) wabi_cont_done) {
    cont->next_prompt = (wabi_word) wabi_copy_val(vm, (wabi_val) WABI_WORD_VAL(cont->next_prompt));
  }
  WABI_SET_TAG(cont, wabi_tag_cont_prompt);
  vm->stor.scan+=WABI_CONT_PROMPT_SIZE;
}


static inline void
wabi_collect_vector_digit(wabi_vm vm, wabi_vector_digit d)
{
  wabi_size j, n;
  wabi_val t;
  n = wabi_vector_digit_node_size(d);
  t = wabi_vector_digit_table(d);
  for(j = 0; j < n; j++)
    *(t + j) = (wabi_word) wabi_copy_val(vm, (wabi_word*) *(t + j));

  vm->stor.scan+= n + WABI_VECTOR_DIGIT_SIZE;
}


static inline void
wabi_collect_vector_deep(wabi_vm vm, wabi_vector_deep d)
{
  wabi_vector m;
  wabi_vector_digit l, r;

  l = wabi_vector_deep_left(d);
  m = wabi_vector_deep_middle(d);
  r = wabi_vector_deep_right(d);

  d->left = (wabi_word) wabi_copy_val(vm, (wabi_word*) l);
  d->middle = (wabi_word) wabi_copy_val(vm, (wabi_word*) m);
  d->right = (wabi_word) wabi_copy_val(vm, (wabi_word*) r);

  vm->stor.scan += WABI_VECTOR_DEEP_SIZE;
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
    wabi_collect_symbol(vm, (wabi_symbol) val);
    break;

  case wabi_tag_atom:
    wabi_collect_atom(vm, (wabi_atom) val);
    break;

  case wabi_tag_bin_leaf:
    vm->stor.scan += WABI_BINARY_LEAF_SIZE;
    break;

  case wabi_tag_pair:
    wabi_collect_val_size(vm, val, WABI_PAIR_SIZE);
    break;

  case wabi_tag_map_entry:
    wabi_collect_val_size(vm, val, WABI_MAP_SIZE);
    break;

  case wabi_tag_map_array:
    wabi_collect_map_array(vm, (wabi_map_array) val);
    break;

  case wabi_tag_map_hash:
    wabi_collect_map_hash(vm, (wabi_map_hash) val);
    break;

  case wabi_tag_oper:
  case wabi_tag_app:
    wabi_collect_val_size(vm, val, WABI_COMBINER_DERIVED_SIZE);
    break;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wabi_collect_combiner_bt(vm, (wabi_combiner_builtin) val);
    break;

  case wabi_tag_ct:
    wabi_collect_val_size(vm, val, WABI_COMBINER_CONTINUATION_SIZE);
    break;

  case wabi_tag_env:
    wabi_collect_env(vm, (wabi_env) val);
    break;

  case wabi_tag_cont_prompt:
    wabi_collect_prompt(vm, (wabi_cont_prompt) val);
    break;

  case wabi_tag_cont_eval:
    wabi_collect_val_size(vm, val, WABI_CONT_EVAL_SIZE);
    break;

  case wabi_tag_cont_apply:
    wabi_collect_val_size(vm, val, WABI_CONT_APPLY_SIZE);
    break;

  case wabi_tag_cont_call:
    wabi_collect_val_size(vm, val, WABI_CONT_CALL_SIZE);
    break;

  case wabi_tag_cont_def:
    wabi_collect_val_size(vm, val, WABI_CONT_DEF_SIZE);
    break;

  case wabi_tag_cont_prog:
    wabi_collect_val_size(vm, val, WABI_CONT_PROG_SIZE);
    break;

  case wabi_tag_cont_args:
    wabi_collect_val_size(vm, val, WABI_CONT_ARGS_SIZE);
    break;

  case wabi_tag_cont_sel:
    wabi_collect_val_size(vm, val, WABI_CONT_SEL_SIZE);
    break;

  case wabi_tag_place:
    wabi_place_val_set((wabi_place) val, wabi_copy_val(vm, wabi_place_val((wabi_place) val)));
    vm->stor.scan+= WABI_PLACE_SIZE;
    break;

  case wabi_tag_vector_digit:
    wabi_collect_vector_digit(vm, (wabi_vector_digit) val);
    break;

  case wabi_tag_vector_deep:
    wabi_collect_vector_deep(vm, (wabi_vector_deep) val);
    break;

  case wabi_tag_forward:
    *val = WABI_WORD_VAL(*val);
    vm->stor.scan++;
    break;

  case wabi_tag_bin_node:
    vm->ert = wabi_error_other;
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
