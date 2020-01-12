#define wabi_store_c

#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include "wabi_store.h"
#include "wabi_word.h"
#include "wabi_binary.h"
#include "wabi_map.h"

static const wabi_word* wabi_store_limit = (wabi_word *)0x07FFFFFFFFFFFFFF;

static const double wabi_low_threshold = 0.11;

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


static inline wabi_size
wabi_store_env_size(wabi_env env)
{
  return WABI_ENV_SIZE + env->numE * WABI_ENV_PAIR_SIZE;
}


wabi_word*
wabi_store_copy_val(wabi_store store, wabi_word *src)
{
  wabi_word* res;
  wabi_size size;

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
    wordcopy(res, src, 2);
    store->heap += 2;
    break;

  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
    wordcopy(res, src, 3);
    store->heap += 3;
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
  case wabi_tag_cont_eval_more:
  case wabi_tag_cont_sel:
    wordcopy(res, src, 4);
    store->heap += 4;
    break;

  case wabi_tag_env:
    size = ((wabi_env) src)->numE * WABI_ENV_PAIR_SIZE;
    wordcopy(res, src, WABI_ENV_SIZE);
    wordcopy(res + WABI_ENV_SIZE, (wabi_word*) ((wabi_env) src)->data, size);
    ((wabi_env) res)->data = (wabi_word) (res + WABI_ENV_SIZE);
    store->heap += WABI_ENV_SIZE + size;
    break;
  }

  *src = (wabi_word) res;
  WABI_SET_TAG(src, wabi_tag_forward);
  return res;
}

static inline void
wabi_store_collect_env(wabi_store store, wabi_env env)
{
  wabi_size j;
  wabi_word *k, *v;

  if(WABI_WORD_VAL(env->prev)) {
    env->prev = (wabi_word) wabi_store_copy_val(store, (wabi_val) WABI_WORD_VAL(env->prev));
    WABI_SET_TAG(env, wabi_tag_env);
  }
  env->maxE = env->numE;

  for(j = 0; j < env->numE; j++) {
    k = ((wabi_val) env->data) + 2 * j;
    v = ((wabi_val) env->data) + 1 + 2 * j;

    *k = (wabi_word) wabi_store_copy_val(store, (wabi_val) *k);
    *v = (wabi_word) wabi_store_copy_val(store, (wabi_val) *v);
  }
}

void
wabi_store_collect_heap(wabi_store store)
{
  wabi_word *scan, size;
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
      scan+=WABI_SYMBOL_SIZE;
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
      wordcopy(store->heap, (wabi_word*) WABI_WORD_VAL(*scan), 2 * size);
      *scan = (wabi_word) store->heap;
      store->heap += 2 * size;
      WABI_SET_TAG(scan, wabi_tag_map_array);
      scan += 2;
      break;

    case wabi_tag_map_hash:
      size = WABI_MAP_BITMAP_COUNT(*(scan + 1));
      wordcopy(store->heap, (wabi_word*) WABI_WORD_VAL(*scan), 2 * size);
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
      wabi_store_collect_env(store, (wabi_env) scan);
      scan += wabi_store_env_size((wabi_env) scan);
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

static inline
wabi_word
wabi_store_used(wabi_store store)
{
  return (store->heap - store->space);
}

void
wabi_store_collect_resize(wabi_store store)
{
  wabi_size used, new_size;
  wabi_word *new_space;

  used = wabi_store_used(store);

  new_size = (wabi_size) ceil(used / wabi_low_threshold);
  new_space = realloc(store->space, WABI_WORD_SIZE * new_size);
  if(new_space != store->space) {
    fprintf(stderr, "resizing has moved heap around.\n");
    exit(1);
  }
  store->limit = store->space + new_size;
  store->size = new_size;

  free(store->old_space);
  store->old_space = NULL;
}


wabi_word*
wabi_store_collect_prepare(wabi_store store)
{
  wabi_word *new_space, *old_space;
  wabi_size size3;

  // printf("Before collection %i over %lu\n", wabi_store_used(store), store->size);
  size3 = (wabi_size) ceil(store->size / wabi_low_threshold);
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

  // printf("After collection %i over %lu\n", wabi_store_used(store), store->size);
  return 1;
}
