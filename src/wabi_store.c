#define wabi_store_c

#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include "wabi_store.h"
#include "wabi_word.h"
#include "wabi_binary.h"

#include "wabi_map.h"
#include "wabi_pair.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_pr.h"


static const wabi_word* wabi_store_limit = (wabi_word *)0x07FFFFFFFFFFFFFF;

// static const double wabi_low_threshold = 0.0013;

static const double wabi_invlow_threshold = 50;

int
wabi_store_init(wabi_store store,
                wabi_size size)
{
  wabi_word *mem = malloc(size * WABI_WORD_SIZE);
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
  case wabi_tag_cont_eval:
    wordcopy(res, src, 2);
    store->heap += 2;
    break;

  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    wordcopy(res, src, 3);
    store->heap += 3;
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
  case wabi_tag_cont_args:
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
      if(*(scan + 2)) {
        *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      }
      scan += 3;
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
      WABI_SET_TAG(scan, wabi_tag_cont_eval);
      scan += 1;
      break;

    case wabi_tag_cont_prompt:
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

    case wabi_tag_cont_args:
      if(WABI_WORD_VAL(*scan)) {
        *scan = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*scan));
      }
      *(scan + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 1));
      *(scan + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 2));
      *(scan + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *(scan + 3));
      WABI_SET_TAG(scan, wabi_tag_cont_args);
      scan += 3;
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

  free(store->old_space);

  used = wabi_store_used(store);
  new_size = (wabi_size) ceil(used * wabi_invlow_threshold);
  new_space = realloc(store->space, WABI_WORD_SIZE * new_size);
  if(new_space != store->space) {
    fprintf(stderr, "resizing has moved heap around.\n");
    exit(1);
  }

  store->limit = store->space + new_size;
  store->size = new_size;
  store->old_space = NULL;
}


wabi_word*
wabi_store_collect_prepare(wabi_store store)
{
  wabi_word *new_space, *old_space;
  wabi_size size3;

  // printf("Before collection %i over %lu\n", wabi_store_used(store), store->size);
  size3 = (wabi_size) ceil(store->size * wabi_invlow_threshold);
  old_space = store->space;
  new_space = (wabi_word*) malloc(size3 * WABI_WORD_SIZE);
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


int
wabi_store_check(wabi_store store, wabi_val val)
{
  wabi_pair pair;
  wabi_map_entry entry;
  wabi_map_array array;
  wabi_map_hash hash;
  wabi_map_entry table;
  wabi_size size;
  wabi_size j;
  wabi_combiner_derived comb;
  wabi_combiner_builtin combb;
  wabi_env env;
  wabi_val data;
  wabi_cont_eval cont_eval;
  wabi_cont_prompt cont_prompt;
  wabi_cont_apply cont_apply;
  wabi_cont_call cont_call;
  wabi_cont_sel cont_sel;
  wabi_cont_args cont_args;
  wabi_cont_def cont_def;
  wabi_cont_prog cont_prog;

  if(val == NULL) {
    return 1;
  }
  if (val < store->space || val >=store->limit) {
    printf("DANGLING POINTER FOUND: %s\n", wabi_tag_to_string(val));
    return 0;
  }
  switch(WABI_TAG(val)) {
  case wabi_tag_fixnum:
  case wabi_tag_constant:
    return 1;

  case wabi_tag_symbol:
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(*val));

  case wabi_tag_pair:
    pair = (wabi_pair) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(pair->cdr))
      && wabi_store_check(store, (wabi_val) pair->car);

  case wabi_tag_map_entry:
    entry = (wabi_map_entry) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(entry->value))
      && wabi_store_check(store, (wabi_val) entry->key);

  case wabi_tag_map_array:
    array = (wabi_map_array) val;
    table = (wabi_map_entry) WABI_WORD_VAL(array->table);
    size = array->size;
    for(j = 0; j < size; j++) {
      if(!wabi_store_check(store, (wabi_val) table + size)) {
        return 0;
      }
    }
    return 1;

  case wabi_tag_map_hash:
    hash = (wabi_map_hash) val;
    table = (wabi_map_entry) WABI_WORD_VAL(hash->table);
    size = WABI_POPCNT(hash->bitmap);
    for(j = 0; j < size; j++) {
      if(!wabi_store_check(store, (wabi_val) table + size)) {
        return 0;
      }
    }
    return 1;

  case wabi_tag_oper:
  case wabi_tag_app:
    comb = (wabi_combiner_derived) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(comb->static_env))
      && wabi_store_check(store, (wabi_val) comb->caller_env_name)
      && wabi_store_check(store, (wabi_val) comb->parameters)
      && wabi_store_check(store, (wabi_val) comb->body);


  case wabi_tag_bt_oper:
  case wabi_tag_bt_app:
    combb = (wabi_combiner_builtin) val;
    if(!combb->c_xtra) {
      return wabi_store_check(store, (wabi_val) combb->c_name);
    } else {
      return wabi_store_check(store, (wabi_val) combb->c_name)
        && wabi_store_check(store, (wabi_val) combb->c_xtra);
    }
  case wabi_tag_env:
    env = (wabi_env) val;
    if(! wabi_store_check(store, (wabi_val) WABI_WORD_VAL(env->prev))) {
      return 0;
    }
    size = env->numE;
    data = (wabi_val) env->data;
    for(j = 0; j < size; j++) {
      if(! wabi_store_check(store, data + 2 * j)) {
        return 0;
      }
      if(! wabi_store_check(store, data + 2 * j + 1)) {
        return 0;
      }
    }
    return 1;

  case wabi_tag_cont_eval:
    cont_eval = (wabi_cont_eval) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_eval->next));

  case wabi_tag_cont_prompt:
    cont_prompt = (wabi_cont_prompt) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_prompt->next))
      && wabi_store_check(store, (wabi_val) cont_prompt->tag);

  case wabi_tag_cont_call:
    cont_call = (wabi_cont_call) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_call->next))
      && wabi_store_check(store, (wabi_val) cont_call->env)
      && wabi_store_check(store, (wabi_val) cont_call->combiner);

  case wabi_tag_cont_def:
    cont_def = (wabi_cont_def) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_def->next))
      && wabi_store_check(store, (wabi_val) cont_def->env)
      && wabi_store_check(store, (wabi_val) cont_def->pattern);

  case wabi_tag_cont_prog:
    cont_prog = (wabi_cont_prog) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_prog->next))
      && wabi_store_check(store, (wabi_val) cont_prog->env)
      && wabi_store_check(store, (wabi_val) cont_prog->expressions);

  case wabi_tag_cont_args:
    cont_args = (wabi_cont_args) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_args->next))
      && wabi_store_check(store, (wabi_val) cont_args->env)
      && wabi_store_check(store, (wabi_val) cont_args->data)
      && wabi_store_check(store, (wabi_val) cont_args->done);

  case wabi_tag_cont_apply:
    cont_apply = (wabi_cont_apply) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_apply->next))
      && wabi_store_check(store, (wabi_val) cont_apply->env)
      && wabi_store_check(store, (wabi_val) cont_apply->args);

  case wabi_tag_cont_sel:
    cont_sel = (wabi_cont_sel) val;
    return wabi_store_check(store, (wabi_val) WABI_WORD_VAL(cont_sel->next))
      && wabi_store_check(store, (wabi_val) cont_sel->env)
      && wabi_store_check(store, (wabi_val) cont_sel->left)
      && wabi_store_check(store, (wabi_val) cont_sel->right);

  case wabi_tag_bin_node:
    return 0;
  case wabi_tag_bin_leaf:
    return 1;

  default:
    return 0;
  }
}
