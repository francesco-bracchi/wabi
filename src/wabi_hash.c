#define wabi_hash_c

#include "wabi_value.h"
#include "wabi_hash.h"
#include "wabi_pair.h"
#include "wabi_binary.h"

typedef struct wabi_hash_state_struct
{
  wabi_word a;
  wabi_word b;
  wabi_word v_hash;
  int err;
} wabi_hash_state_t;

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
  for(wabi_word j = 0; j < size; j++) {
    state->v_hash = state->a * state->v_hash + *(data + j);
    state->a *= state->b;
  }
}

void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val);

void
wabi_hash_binary(wabi_hash_state_t *state, wabi_binary bin);

void
wabi_binary_leaf_hash(wabi_hash_state_t *state, wabi_binary_leaf leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, WABI_WORD_VAL(leaf->length));
}

void
wabi_binary_node_hash(wabi_hash_state_t *state, wabi_binary_node node)
{
  wabi_hash_binary(state, (wabi_binary) node->left);
  wabi_hash_binary(state, (wabi_binary) node->right);
}


void
wabi_hash_binary(wabi_hash_state_t *state, wabi_binary bin)
{
  // todo: make this if
  if(WABI_TAG(bin) == wabi_tag_bin_leaf) {
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    return;
  }
  wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
}

/* void */
/* wabi_hash_entry(wabi_hash_state_t *state, wabi_map_entry entry) */
/* { */
/*   wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry)); */
/*   wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry)); */
/* } */

/* void */
/* wabi_hash_map(wabi_hash_state_t *state, wabi_map map) */
/* { */
/*   wabi_map_entry entry; */
/*   wabi_map_iter_t iter; */
/*   wabi_map_iterator_init(&iter, map); */
/*   while((entry = wabi_map_iterator_current(&iter))) { */
/*     wabi_hash_entry(state, entry); */
/*     wabi_map_iterator_next(&iter); */
/*   } */
/* } */

void
wabi_hash_val(wabi_hash_state_t *state, wabi_val val)
{
  wabi_word tag = WABI_TAG(val);

  if(tag < wabi_tag_immediate_limit) {
    wabi_hash_step(state, (char *) val, 8);
    return;
  }
  switch(tag) {
  case wabi_tag_pair:
    wabi_hash_step(state, "P", 1);
    wabi_hash_val(state, wabi_car((wabi_pair) val));
    wabi_hash_val(state, wabi_cdr((wabi_pair) val));
    return;
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_hash_step(state, "B", 1);
    wabi_hash_binary(state, (wabi_binary) val);
    return;
  /* case WABI_TYPE_MAP: */
  /*   wabi_hash_step(state, "M", 1); */
  /*   wabi_hash_map(state, (wabi_map) val); */
  /*   return; */
  /* case WABI_TYPE_SYMBOL: */
  /*   wabi_hash_step(state, "S", 1); */
  /*   wabi_hash_binary(state, wabi_symbol_binary((wabi_symbol) val)) */
  /*   return; */
  /* } */
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
