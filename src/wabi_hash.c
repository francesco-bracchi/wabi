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


void
wabi_hash_step(wabi_hash_state_t *state, char *data, wabi_word_t size)
{
  for(wabi_word_t j = 0; j < size; j++) {
    state->v_hash = state->a * state->v_hash + *(data + j);
    state->a *= state->b;
  }
}


void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val);


void
wabi_hash_binary(wabi_hash_state_t *state, wabi_val bin);


void
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


void
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


void
wabi_hash_map(wabi_hash_state_t *state, wabi_map_table map)
{
  if(wabi_val_is_map_array((wabi_val) map)) {
    wabi_word_t size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
    wabi_map_table table = (wabi_map_table) WABI_MAP_ARRAY_TABLE((wabi_map_array) map);
    for(wabi_word_t offset = 0; offset < size; offset++)
      wabi_hash_val(state, (wabi_val) (table + offset));
  }
  if(wabi_val_is_map_hash((wabi_val) map)) {
    wabi_word_t bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
    wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
    wabi_map_table table = (wabi_map_table) WABI_MAP_HASH_TABLE((wabi_map_hash) map);
    for(wabi_word_t offset = 0; offset < size; offset++)
      wabi_hash_val(state, (wabi_val) (table + offset));
  }
}


void
wabi_hash_entry(wabi_hash_state_t *state, wabi_map_entry entry)
{
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry));
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry));
}


void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val)
{
  wabi_word_t tag = wabi_val_tag(val);
  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    wabi_hash_step(state, (char *) val, 4);
    return;
  }
  switch(tag) {
  case WABI_TAG_PAIR:
    wabi_hash_step(state, "P", 1);
    wabi_hash_val(state, (wabi_val) (*val & WABI_VALUE_MASK));
    wabi_hash_val(state, (wabi_val) (*(val + 1) & WABI_VALUE_MASK));
    return;
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    wabi_hash_step(state, "B", 1);
    wabi_hash_binary(state, val);
    return;
  case WABI_TAG_MAP_ARRAY:
  case WABI_TAG_MAP_HASH:
    wabi_hash_step(state, "M", 1);
    wabi_hash_map(state, (wabi_map_table) val);
    return;
  case WABI_TAG_SYMBOL:
    wabi_hash_step(state, "S", 1);
    wabi_hash_binary(state, (wabi_val) (*val & WABI_VALUE_MASK));
    return;
  case WABI_TAG_FORWARD:
    wabi_hash_val(state, (wabi_val) wabi_val_value(val));
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
