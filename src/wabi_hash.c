#define wabi_hash_c

#include "wabi_value.h"
#include "wabi_hash.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_place.h"

void
wabi_hash_state_init(wabi_hash_state_t* state)
{
  state->a = 3141527183;
  state->b = 2718331415;
  state->v_hash = 179;
  state->err = 0;
}

void
wabi_hash_step(wabi_hash_state_t *state,
               char *data,
               wabi_word size)
{
  wabi_word j;
  for(j = 0; j < size; j++) {
    state->v_hash = state->a * state->v_hash + *(data + j);
    state->a *= state->b;
  }
}

void
wabi_hash_combiner(wabi_hash_state_t *state, wabi_combiner c)
{
  switch(WABI_TAG(c)) {
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_builtin) c)->c_name);
    return;
  /* case wabi_tag_app: */
  /* case wabi_tag_oper: */
  default:
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_derived) c)->static_env);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_derived) c)->caller_env_name);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_derived) c)->parameters);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_derived) c)->body);
  }
}

void
wabi_hash_val(wabi_hash_state state, wabi_val val)
{
  switch(WABI_TAG(val)) {
  case wabi_tag_constant:
  case wabi_tag_fixnum:
    wabi_hash_step(state, (char *) val, 8);
    return;
  case wabi_tag_symbol:
    wabi_hash_step(state, "S", 1);
    wabi_hash_val(state, wabi_symbol_to_binary((wabi_symbol) val));
    return;
  case wabi_tag_pair:
    wabi_pair_hash(state, (wabi_pair) val);
    return;
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_binary_hash(state, (wabi_binary) val);
    return;
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    wabi_map_hash_(state, (wabi_map) val);
    return;
  case wabi_tag_env:
    wabi_env_hash(state, (wabi_env) val);
    return;
  case wabi_tag_app:
  case wabi_tag_bt_app:
  case wabi_tag_oper:
  case wabi_tag_bt_oper:
    wabi_hash_step(state, "C", 1);
    wabi_hash_step(state, (char*) val, 1);
    wabi_hash_combiner(state, (wabi_combiner) val);
    return;

  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_sel:
  case wabi_tag_cont_args:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    wabi_cont_hash(state, (wabi_cont) val);
    return;
  case wabi_tag_place:
    wabi_place_hash(state, (wabi_place) val);
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
