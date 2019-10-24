
#define wabi_store_c

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <math.h>
#include "wabi_store.h"
#include "wabi_word.h"
#include "wabi_binary.h"
#include "wabi_map.h"

static const wabi_word* wabi_store_limit = (wabi_word *)0x07FFFFFFFFFFFFFF;

static const double wabi_low_threshold = 0.2;

int
wabi_store_init(wabi_store store,
                wabi_size size)
{
  wabi_word *mem = malloc(WABI_WORD_SIZE * size);
  if(mem && (mem + size <= wabi_store_limit)) {
    store->space = mem;
    store->limit = mem + size;
    store->heap = mem;
    store->size = size;
    return 1;
  }
  return 0;
}

void
wabi_store_destroy(wabi_store store)
{
  free(store->space);
}

void
wabi_store_compact_binary(wabi_store store, wabi_val src)
{
  wabi_size len, word_size;

  wabi_binary_leaf new_leaf;
  wabi_word *new_blob;
  char *data;

  len = wabi_binary_length((wabi_binary) src);
  word_size = wabi_binary_word_size(len);
  new_leaf = (wabi_binary_leaf) store->heap;
  store->heap += 2;
  new_blob = (wabi_word *) store->heap;
  store->heap += 1 + word_size;
  new_leaf->length = len;
  new_leaf->data_ptr = (wabi_word) (new_blob+1);
  *new_blob = 1 + word_size;
  WABI_SET_TAG(new_blob, wabi_tag_bin_blob);
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

  wabi_binary_memcopy((char*)(new_blob + 1), (wabi_binary) src);
}


wabi_word*
wabi_store_copy_val(wabi_store store, wabi_word *src)
{
  wabi_word* res;
  res = store->heap;
  switch(WABI_TAG(src)) {

  /* case wabi_tag_var: */
  /* case wabi_tag_alien: */
  /* case wabi_tag_tagged: */
  /* case wabi_tag_bin_blob: */
  /*   return res; */

  case wabi_tag_forward:
    return WABI_DEREF(src);

  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    wabi_store_compact_binary(store, src);
    break;

  case wabi_tag_constant:
  case wabi_tag_fixnum:
  case wabi_tag_symbol:
    *res = *src;
    store->heap++;
    break;

  case wabi_tag_pair:
  case wabi_tag_map_array:
  case wabi_tag_map_entry:
  case wabi_tag_map_hash:
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
  case wabi_tag_cont_eval:
  case wabi_tag_env:
    memcpy(res, src, 2 * WABI_WORD_SIZE);
    store->heap += 2;
    break;

  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    memcpy(res, src, 3 * WABI_WORD_SIZE);
    store->heap += 3;
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
  case wabi_tag_cont_eval_more:
  case wabi_tag_cont_sel:
    memcpy(res, src, 4 * WABI_WORD_SIZE);
    store->heap += 4;
    break;
  }

  *src = (wabi_word) res;
  WABI_SET_TAG(src, wabi_tag_forward);
  return res;
}


void
wabi_store_collect_heap(wabi_store store)
{
  wabi_word *scan, size, bitmap;

  scan = store->space;
  do {
    switch(WABI_TAG(scan)) {
    /* case wabi_tag_var: */
    /* case wabi_tag_alien: */
    /* case wabi_tag_tagged: */

    case wabi_tag_bin_blob:
      scan += WABI_WORD_VAL(*scan);
      break;

    case wabi_tag_constant:
    case wabi_tag_fixnum:
      scan++;
      break;

    case wabi_tag_symbol:
      *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      WABI_SET_TAG(scan, wabi_tag_symbol);
      scan++;
      break;

    case wabi_tag_bin_node:
    case wabi_tag_bin_leaf:
      scan += 2;
      break;

    case wabi_tag_pair:
      *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      WABI_SET_TAG(scan, wabi_tag_pair);
      scan += 2;
      break;

    case wabi_tag_map_entry:
      *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      WABI_SET_TAG(scan, wabi_tag_map_entry);
      scan += 2;
      break;

    case wabi_tag_map_array:
      size = *(scan + 1);
      memcpy(store->heap, (wabi_word*) WABI_WORD_VAL(*scan), 2 * WABI_WORD_SIZE * size);
      *scan = (wabi_word) store->heap;
      store->heap += 2 * size;
      WABI_SET_TAG(scan, wabi_tag_map_array);
      scan += 2;
      break;

    case wabi_tag_map_hash:
      size = WABI_MAP_BITMAP_COUNT(*(scan + 1));
      memcpy(store->heap, (wabi_word*) WABI_WORD_VAL(*scan), 2 * WABI_WORD_SIZE * size);
      *scan = (wabi_word) store->heap;
      store->heap += 2 * size;
      WABI_SET_TAG(scan, wabi_tag_map_hash);
      scan += 2;
      break;

    case wabi_tag_bt_app:
    case wabi_tag_bt_oper:
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      scan += 2;
      break;

    case wabi_tag_oper:
      *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      *(scan + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 3));
      WABI_SET_TAG(scan, wabi_tag_oper);
      scan += 4;
      break;

    case wabi_tag_app:
      *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      *(scan + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 3));
      WABI_SET_TAG(scan, wabi_tag_app);
      scan += 4;
      break;

    case wabi_tag_env:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      WABI_SET_TAG(scan, wabi_tag_env);
      scan += 2;
      break;

    case wabi_tag_cont_eval:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      WABI_SET_TAG(scan, wabi_tag_cont_eval);
      scan += 2;
      break;


    case wabi_tag_cont_apply:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      WABI_SET_TAG(scan, wabi_tag_cont_apply);
      scan += 3;
      break;

    case wabi_tag_cont_call:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      WABI_SET_TAG(scan, wabi_tag_cont_call);
      scan += 3;
      break;

    case wabi_tag_cont_def:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      WABI_SET_TAG(scan, wabi_tag_cont_def);
      scan += 3;
      break;

    case wabi_tag_cont_prog:
      // todo: unify all these cases
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      WABI_SET_TAG(scan, wabi_tag_cont_prog);
      scan += 3;
      break;

    case wabi_tag_cont_eval_more:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      *(scan + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 3));
      WABI_SET_TAG(scan, wabi_tag_cont_eval_more);
      scan += 4;
      break;

    case wabi_tag_cont_sel:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      *(scan + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 3));
      WABI_SET_TAG(scan, wabi_tag_cont_sel);
      scan += 4;
      break;

    case wabi_tag_forward:
      *scan = WABI_WORD_VAL(*scan);
      scan++;
      break;
    }
  }
  while(scan < store->heap);
}

void
wabi_store_collect_resize(wabi_store store)
{
  /* wabi_size used, new_size; */
  /* wabi_word *new_space; */

  /* used = store->heap - store->space; */

  /* new_size = (wabi_size) ceil(used / wabi_low_threshold); */

  /* new_space = realloc(store->space, new_size); */
  /* store->limit = new_space + new_size; */
  /* store->size = new_size; */
  /* store->heap = new_space + used; */
  /* store->space = new_space; */

  free(store->old_space);
  store->old_space = NULL;
}


static inline
int
wabi_store_used(wabi_store store)
{
  return (store->heap - store->space);
}


wabi_word*
wabi_store_collect_prepare(wabi_store store)
{
  wabi_word *new_space, *old_space;
  wabi_size size3;
  int j;

  // printf("Before collection %i\n", wabi_store_used(store));
  size3 = store->size;
  old_space = store->space;
  new_space = (wabi_word*) malloc(WABI_WORD_SIZE * size3);
  if(new_space && (new_space + size3 <= wabi_store_limit)) {
    store->space = new_space;
    store->limit = new_space + size3;
    store->heap = new_space;
    store->size = size3;
    store->old_space = old_space;
    return new_space;
  }
  return NULL;
}


int
wabi_store_collect(wabi_store store)
{
  wabi_store_collect_heap(store);
  wabi_store_collect_resize(store);
  return 1;
}
