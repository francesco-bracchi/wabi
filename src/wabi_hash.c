#define wabi_hash_c

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_hash.h"
#include "wabi_atomic.h"
#include "stdio.h"

void
wabi_hash_state_init(wabi_hash_state_t* state)
{
  state->a = 3141527183;
  state->b = 2718331415;
  state->v_hash = 179;
}


static inline void
wabi_hash_step(wabi_hash_state_t *state, char *data, wabi_word_t size)
{
  for(wabi_word_t j = 0; j < size; j++) {
    state->v_hash = state->a * state->v_hash + *(data + j);
    state->a *= state->b;
  }
}


static void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val);


static void
wabi_hash_binary(wabi_hash_state_t *state, wabi_val bin);


static inline void
wabi_binary_leaf_hash(wabi_hash_state_t *state, wabi_binary_leaf_t* leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, leaf->length & WABI_VALUE_MASK);
}


void
wabi_binary_node_hash(wabi_hash_state_t *state, wabi_binary_node_t* node)
{
  wabi_hash_binary(state, (wabi_val) node->left);
  wabi_hash_binary(state, (wabi_val) node->right);
}


static void
wabi_hash_binary(wabi_hash_state_t *state, wabi_val bin)
{
  // todo: make this if
  switch(wabi_val_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    return;
  default:
    wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
  }
}


static inline void
wabi_hash_entry(wabi_hash_state_t *state, wabi_map_entry entry)
{
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry));
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry));
}


static inline void
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


static void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val)
{
  wabi_word_t type = wabi_val_type(val);
  if(type <= WABI_TAG_ATOMIC_LIMIT) {
    wabi_hash_step(state, (char *) val, 4);
    return;
  }
  switch(type) {

  case WABI_TYPE_PAIR:
    wabi_hash_step(state, "P", 1);
    wabi_hash_val(state, (wabi_val) (*val & WABI_VALUE_MASK));
    wabi_hash_val(state, (wabi_val) (*(val + 1) & WABI_VALUE_MASK));
    return;
  case WABI_TYPE_BIN:
    wabi_hash_step(state, "B", 1);
    wabi_hash_binary(state, val);
    return;
  case WABI_TYPE_MAP:
    wabi_hash_step(state, "M", 1);
    wabi_hash_map(state, (wabi_map) val);
    return;
  case WABI_TYPE_SYMBOL:
    wabi_hash_step(state, "S", 1);
    wabi_hash_binary(state, (wabi_val) (*val & WABI_VALUE_MASK));
    return;
  }
}


wabi_word_t
wabi_hash_raw(wabi_val val)
{
  wabi_hash_state_t hash_state;
  wabi_hash_state_init(&hash_state);
  wabi_hash_val(&hash_state, val);
  return hash_state.v_hash & WABI_VALUE_MASK;
}


wabi_val
wabi_hash(wabi_vm vm, wabi_val val)
{
  wabi_word_t hash = wabi_hash_raw(val);
  return wabi_smallint(vm, hash & WABI_VALUE_MASK);
}
