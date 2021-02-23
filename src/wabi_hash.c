#define wabi_hash_c

#include "wabi_value.h"
#include "wabi_hash.h"
#include "wabi_list.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_place.h"
#include "wabi_vector.h"


static inline void
wabi_hash_state_init(const wabi_hash_state state)
{
  state->a = 3141527183;
  state->b = 2718331415;
  state->v_hash = 179;
  state->err = 0;
}

static void
wabi_hash_binary_generic(const wabi_hash_state state, const wabi_binary bin);

static inline void
wabi_binary_leaf_hash(const wabi_hash_state state, const wabi_binary_leaf leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, wabi_binary_length((wabi_binary) leaf));
}

static inline void
wabi_binary_node_hash(const wabi_hash_state state, const wabi_binary_node node)
{
  wabi_hash_binary_generic(state, (wabi_binary) node->left);
  wabi_hash_binary_generic(state, (wabi_binary) node->right);
}

static void
wabi_hash_binary_generic(const wabi_hash_state state, const wabi_binary bin)
{
  switch(WABI_TAG(bin)) {
  case wabi_tag_bin_leaf:
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    break;
  case wabi_tag_bin_node:
    wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
    break;
  }
}

static inline void
wabi_hash_map_entry(const wabi_hash_state state,
                    const wabi_map_entry entry)
{
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry));
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry));
}

static inline void
wabi_hash_map_generic(const wabi_hash_state state,
                       const wabi_map map)
{
  wabi_map_entry entry;
  wabi_map_iter_t iter;
  wabi_map_iterator_init(&iter, map);
  while((entry = wabi_map_iterator_current(&iter))) {
    wabi_hash_map_entry(state, entry);
    wabi_map_iterator_next(&iter);
  }
}

static inline void
wabi_hash_map(const wabi_hash_state state, const wabi_map map)
{
  if(WABI_IS(wabi_tag_map_entry, map)) {
    wabi_hash_map_entry(state, (wabi_map_entry) map);
    return;
  }
  wabi_hash_map_generic(state, map);
}

static inline void
wabi_hash_vector(const wabi_hash_state state,
                 const wabi_vector v)
{
  wabi_vector_iter_t iter;
  wabi_val x;

  wabi_vector_iter_init(&iter, v);
  for(;;) {
    x = wabi_vector_iter_current(&iter);
    if(! x) return;
    wabi_hash_val(state, x);
    wabi_vector_iter_next(&iter);
  }
}

static inline void
wabi_hash_cont(const wabi_hash_state state,
               const wabi_cont cont0)
{
  wabi_cont cont;
  cont = cont0;
  wabi_hash_step(state, "O", 1);
  while(cont) {
    switch(WABI_TAG(cont)) {
    case wabi_tag_cont_eval:
      wabi_hash_step(state, "E", 1);
      break;
    case wabi_tag_cont_prompt:
      wabi_hash_step(state, "P", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prompt) cont)->tag);
      break;
    case wabi_tag_cont_apply:
      wabi_hash_step(state, "A", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_apply) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_apply) cont)->args);
      break;
    case wabi_tag_cont_call:
      wabi_hash_step(state, "C", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_call) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_call) cont)->combiner);
      break;
    case wabi_tag_cont_sel:
      wabi_hash_step(state, "S", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->left);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_sel) cont)->right);
      break;
    case wabi_tag_cont_args:
      wabi_hash_step(state, "R", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->data);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_args) cont)->done);
      break;
    case wabi_tag_cont_def:
      wabi_hash_step(state, "D", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_def) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_def) cont)->pattern);
      break;
    case wabi_tag_cont_prog:
      wabi_hash_step(state, "P", 1);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prog) cont)->env);
      wabi_hash_val(state, (wabi_val) ((wabi_cont_prog) cont)->expressions);
      break;
    default:
      break;
    }
    cont = wabi_cont_pop(cont);
  }
}

void
wabi_hash_val(wabi_hash_state state, wabi_val val)
{
  switch(WABI_TAG(val)) {
  case wabi_tag_fixnum:
    wabi_hash_step(state, (char *) val, 8);
    return;

  case wabi_tag_atom:
    wabi_hash_step(state, "A", 1);
    wabi_hash_val(state, wabi_atom_to_binary(val));
    return;

  case wabi_tag_symbol:
    wabi_hash_step(state, "S", 1);
    wabi_hash_val(state, wabi_symbol_to_binary(val));
    return;

  case wabi_tag_pair:
    wabi_hash_step(state, "P", 1);
    wabi_hash_val(state, wabi_car((wabi_pair) val));
    wabi_hash_val(state, wabi_cdr((wabi_pair) val));
    return;

  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_hash_step(state, "B", 1);
    wabi_hash_binary_generic(state, (wabi_binary) val);
    return;

  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    wabi_hash_step(state, "M", 1);
    wabi_hash_map(state, (wabi_map) val);
    return;

  case wabi_tag_vector_digit:
  case wabi_tag_vector_deep:
  wabi_hash_step(state, "V", 1);
    wabi_hash_vector(state, (wabi_vector) val);
    return;

  case wabi_tag_env:
    wabi_hash_step(state, "E", 1);
    wabi_hash_step(state, (char*) ((wabi_env) val)->uid, WABI_WORD_SIZE);
    return;

  case wabi_tag_app:
    wabi_hash_step(state, "CDA", 3);
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_static_env((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_caller_env_name((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_parameters((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_body((wabi_combiner_derived) val));
    return;

  case wabi_tag_oper:
    wabi_hash_step(state, "CDO", 3);
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_static_env((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_caller_env_name((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_parameters((wabi_combiner_derived) val));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_body((wabi_combiner_derived) val));
    return;

  case wabi_tag_bt_app:
    wabi_hash_step(state, "CBA", 3);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_builtin) val)->c_name);
    return;

  case wabi_tag_bt_oper:
    wabi_hash_step(state, "CBO", 3);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_builtin) val)->c_name);
    return;

    // todo rename ct_app to _ct
  case wabi_tag_ct:
    wabi_hash_step(state, "CC", 2);
    wabi_hash_val(state, (wabi_val) wabi_combiner_continuation_cont((wabi_combiner_continuation) val));
    return;

  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_sel:
  case wabi_tag_cont_args:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    wabi_hash_cont(state, (wabi_cont) val);
    return;
  case wabi_tag_place:
    wabi_hash_step(state, "R", 1);
    wabi_hash_step(state, (char*) &(((wabi_place) val)->uid), WABI_WORD_SIZE);
    return;
  }
  state->err = 1;
}


wabi_word
wabi_hash(wabi_val val)
{
  wabi_hash_state_t hash_state;

  wabi_hash_state_init(&hash_state);
  wabi_hash_val(&hash_state, val);
  if(!hash_state.err) {
    return WABI_WORD_VAL(hash_state.v_hash);
  }
  return 0;
}
