#define wabi_hash_c

#include "wabi_value.h"
#include "wabi_hash.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_symbol.h"
#include "wabi_env.h"
#include "wabi_combiner.h"

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
wabi_hash_val(wabi_hash_state_t *state, wabi_val val);

void
wabi_hash_entry(wabi_hash_state_t *state, wabi_map_entry entry)
{
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry));
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry));
}

void
wabi_hash_map(wabi_hash_state_t *state, wabi_map map)
{
  wabi_map_entry entry;
  wabi_map_iter_t iter;
  wabi_map_iterator_init(&iter, map);
  while((entry = wabi_map_iterator_current(&iter))) {
    wabi_hash_entry(state, entry);
    wabi_map_iterator_next(&iter);
  }
}

void
wabi_hash_env(wabi_hash_state_t *state, wabi_env env)
{
  wabi_size j;
  do {
    for(j = 0; j< env->numE; j++) {
      wabi_hash_val(state, (wabi_val) (env->data + j * WABI_ENV_PAIR_SIZE));
      wabi_hash_val(state, (wabi_val) (env->data + 1 + j * WABI_ENV_PAIR_SIZE));
    }
    env = (wabi_env) WABI_WORD_VAL(env->prev);
  } while(env);
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
wabi_hash_val(wabi_hash_state_t *state, wabi_val val)
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
    wabi_hash_step(state, "P", 1);
    wabi_hash_val(state, wabi_car((wabi_pair) val));
    wabi_hash_val(state, wabi_cdr((wabi_pair) val));
    return;
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_hash_step(state, "B", 1);
    wabi_binary_hash(state, (wabi_binary) val);
    return;
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    wabi_hash_step(state, "M", 1);
    wabi_hash_map(state, (wabi_map) val);
    return;
  case wabi_tag_env:
    wabi_hash_step(state, "E", 1);
    wabi_hash_env(state, (wabi_env) val);
    return;
  case wabi_tag_app:
  case wabi_tag_bt_app:
  case wabi_tag_oper:
  case wabi_tag_bt_oper:
    wabi_hash_step(state, "C", 1);
    wabi_hash_step(state, (char*) val, 1);
    wabi_hash_combiner(state, (wabi_combiner) val);
    return;
  }
  // todo: add all the continuation stuff?
  state->err = 1;
}

wabi_word
wabi_hash(wabi_val val)
{
  wabi_hash_state_t hash_state;;
  wabi_hash_state_init(&hash_state);
  wabi_hash_val(&hash_state, val);
  if(!hash_state.err) {
    return WABI_WORD_VAL(hash_state.v_hash);
  }
  return 0;
}
