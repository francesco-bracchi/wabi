#define wabi_hash_c

#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_hash.h"
#include "wabi_atomic.h"

typedef struct wabi_hash_state_struct
{
  wabi_word_t a;
  wabi_word_t b;
  wabi_word_t v_hash;
} wabi_hash_state_t;


void
wabi_hash_state_init(wabi_hash_state_t* state)
{
  state->a = 31415;
  state->b = 27183;
  state->v_hash = 0;
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
wabi_hash_binary(wabi_hash_state_t *state, wabi_obj bin);


void
wabi_binary_leaf_hash(wabi_hash_state_t *state, wabi_binary_leaf_t* leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, leaf->length & WABI_VALUE_MASK);
}


void
wabi_binary_node_hash(wabi_hash_state_t *state, wabi_binary_node_t* node)
{
  wabi_hash_binary(state, (wabi_obj) node->left);
  wabi_hash_binary(state, (wabi_obj) node->right);
}


void
wabi_hash_binary(wabi_hash_state_t *state, wabi_obj bin)
{
  switch(wabi_obj_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    return;
  default:
    wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
  }
}


void
wabi_hash_obj(wabi_hash_state_t *state, wabi_obj obj)
{
  wabi_word_t tag = wabi_obj_tag(obj);
  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    wabi_hash_step(state, (char *) obj, 4);
    return;
  }
  switch(tag) {
  case WABI_TAG_PAIR:
    wabi_hash_step(state, "P", 1);
    wabi_hash_obj(state, (wabi_obj) (*obj & WABI_VALUE_MASK));
    wabi_hash_obj(state, (wabi_obj) (*(obj + 1) & WABI_VALUE_MASK));
    return;
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    wabi_hash_step(state, "B", 1);
    wabi_hash_binary(state, obj);
    return;
  case WABI_TAG_FORWARD:
    wabi_hash_obj(state, (wabi_obj) wabi_obj_value(obj));
    return;
  }
}


void
wabi_hash(wabi_obj obj, wabi_obj *res, wabi_error* err)
{
  wabi_hash_state_t hash_state;
  wabi_hash_state_init(&hash_state);
  wabi_hash_obj(&hash_state, obj);
  wabi_smallint(hash_state.v_hash & WABI_VALUE_MASK, res, err);
}
