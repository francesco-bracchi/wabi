#define wabi_store_c

#include <stdlib.h>

#include <math.h>
#include "wabi_store.h"
#include "wabi_word.h"
#include "wabi_binary.h"

#include "wabi_map.h"
#include "wabi_pair.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_pr.h"


static const wabi_word* wabi_store_limit = (wabi_word *)0x00FFFFFFFFFFFFFF;


int
wabi_store_init(wabi_store store,
                wabi_size size)
{
  wabi_word *new_space = malloc(size * WABI_WORD_SIZE);
  wabi_word *old_space = malloc(size * WABI_WORD_SIZE);
  if(new_space
     && old_space
     && (new_space + size <= wabi_store_limit)
     && (old_space + size <= wabi_store_limit)) {
    memset(new_space, 0, size * WABI_WORD_SIZE);
    memset(old_space, 0, size * WABI_WORD_SIZE);
    store->new_space = new_space;
    store->limit = new_space + size;
    store->heap = new_space;
    store->size = size;
    store->old_space = old_space;
    return 1;
  }
  return 0;
}


void
wabi_store_destroy(wabi_store store)
{
  if(store->new_space) free(store->new_space);
  if(store->old_space) free(store->old_space);
}


wabi_word*
wabi_store_copy_val(wabi_store store, wabi_word *src)
{
  wabi_word* res;
  wabi_size size;

  // printf("copy %s\n", wabi_tag_to_string(store->heap));

  res = store->heap;
  switch(WABI_TAG(src)) {

  /* case wabi_tag_var: */
  /* case wabi_tag_alien: */
  /* case wabi_tag_tagged: */
  /* case wabi_tag_bin_blob: */
  /*   return res; */

  case wabi_tag_forward:
    return WABI_DEREF(src);

  case wabi_tag_constant:
  case wabi_tag_fixnum:
  case wabi_tag_symbol:
    *res = *src;
    store->heap++;
    break;

  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_binary_copy_val(store, (wabi_binary) src);
    break;

  case wabi_tag_pair:
    wabi_pair_copy_val(store, (wabi_pair) src);
    break;

  case wabi_tag_map_array:
  case wabi_tag_map_entry:
  case wabi_tag_map_hash:
    wabi_map_copy_val(store, (wabi_map) src);
    break;

  case wabi_tag_env:
    wabi_env_copy_val(store, (wabi_env) src);
    break;

  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
  case wabi_tag_cont_args:
  case wabi_tag_cont_prompt:
  case wabi_tag_cont_sel:
    wabi_cont_copy_val(store, (wabi_cont) src);
    break;

  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
  case wabi_tag_app:
  case wabi_tag_oper:
  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    wabi_combiner_copy_val(store, (wabi_combiner) src);
    break;
  }

  *src = (wabi_word) res;
  WABI_SET_TAG(src, wabi_tag_forward);
  return res;
}

void
wabi_store_collect_heap(wabi_store store)
{
  wabi_word size;
  store->scan = store->new_space;

  do {
    // printf("collecting %s\n", wabi_tag_to_string(store->scan));
    switch(WABI_TAG((store->scan))) {
    /* case wabi_tag_var: */
    /* case wabi_tag_alien: */
    /* case wabi_tag_tagged: */

    case wabi_tag_bin_blob:
      (store->scan) += WABI_WORD_VAL(*(store->scan));
      break;

    case wabi_tag_constant:
    case wabi_tag_fixnum:
      (store->scan)++;
      break;

    case wabi_tag_symbol:
      *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      WABI_SET_TAG((store->scan), wabi_tag_symbol);
      store->scan+=WABI_SYMBOL_SIZE;
      break;

    case wabi_tag_bin_leaf:
      wabi_binary_collect_val(store, (wabi_binary) store->scan);
      break;

    case wabi_tag_pair:
      wabi_pair_collect_val(store, (wabi_pair) store->scan);
      break;

    case wabi_tag_map_entry:
    case wabi_tag_map_array:
    case wabi_tag_map_hash:
      wabi_map_collect_val(store, (wabi_map) store->scan);
      break;

    case wabi_tag_oper:
    case wabi_tag_app:
    case wabi_tag_bt_app:
    case wabi_tag_bt_oper:
    case wabi_tag_ct_app:
    case wabi_tag_ct_oper:
      wabi_combiner_collect_val(store, (wabi_combiner) store->scan);
      break;

    case wabi_tag_env:
      wabi_env_collect_val(store, (wabi_env) store->scan);
      break;

    case wabi_tag_cont_eval:
    case wabi_tag_cont_prompt:
    case wabi_tag_cont_apply:
    case wabi_tag_cont_call:
    case wabi_tag_cont_def:
    case wabi_tag_cont_prog:
    case wabi_tag_cont_args:
    case wabi_tag_cont_sel:
      wabi_cont_collect_val(store, (wabi_cont) store->scan);
      break;

    case wabi_tag_forward:
      *(store->scan) = WABI_WORD_VAL(*(store->scan));
      (store->scan)++;
      break;
    }
  }
  while((store->scan) < store->heap);
}


static inline
wabi_word
wabi_store_used(wabi_store store)
{
  return (store->heap - store->new_space);
}


static inline
wabi_word
wabi_store_free(wabi_store store)
{
 return (store->limit - store->heap);
}


static inline
double
wabi_store_used_ratio(wabi_store store)
{
  return (double) wabi_store_used(store) / store->size;
}


void
wabi_store_collect_prepare(wabi_store store)
{
  wabi_word *swap;
  swap = store->new_space;
  store->new_space = store->old_space;
  store->old_space = swap;

  store->heap = store->new_space;
  store->limit = store->new_space + store->size;

}


int
wabi_store_collect(wabi_store store)
{
  wabi_store_collect_heap(store);
  return 1;
}
