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


static const wabi_word* wabi_store_limit = (wabi_word *)0x00FFFFFFFFFFFFFF;

// static const double wabi_store_ratio = 10.0;

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


static inline void
wabi_store_trim_cont(wabi_store store, wabi_combiner_continuation src)
{
  /* wabi_word tag; */
  /* wabi_cont cont, cont0, new_cont, new_cont0; */

  /* tag = (wabi_word) WABI_WORD_VAL(src->tag); */
  /* cont = (wabi_cont) src->cont; */

  /* while(1) { */
  /*   if(WABI_IS(wabi_tag_cont_prompt, cont) && ((wabi_cont_prompt) cont)->tag == tag) { */
  /*     if(cont0) cont0->next = wabi_tag_cont_prompt; */
  /*     return; */
  /*   } */
  /*   cont0 = cont; */
  /*   cont = (wabi_cont) WABI_WORD_VAL(cont->next); */
  /* } */
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
    wabi_binary_copy_val(store, (wabi_binary) src);
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

  case wabi_tag_cont_args:
  case wabi_tag_cont_sel:
    wordcopy(res, src, 4);
    store->heap += 4;
    break;

  case wabi_tag_app:
  case wabi_tag_oper:
    wordcopy(res, src, 5);
    store->heap += 5;
    break;

  case wabi_tag_env:
    size = ((wabi_env) src)->numE * WABI_ENV_PAIR_SIZE;
    wordcopy(res, src, WABI_ENV_SIZE);
    wordcopy(res + WABI_ENV_SIZE, (wabi_word*) ((wabi_env) src)->data, size);
    ((wabi_env) res)->data = (wabi_word) (res + WABI_ENV_SIZE);
    store->heap += WABI_ENV_SIZE + size;
    break;
  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    // wabi_store_trim_cont(store, (wabi_combiner_continuation) src);
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
  }
  env->maxE = env->numE;
  for(j = 0; j < env->numE; j++) {
    k = ((wabi_val) env->data) + 2 * j;
    v = ((wabi_val) env->data) + 1 + 2 * j;
    *k = (wabi_word) wabi_store_copy_val(store, (wabi_val) *k);
    *v = (wabi_word) wabi_store_copy_val(store, (wabi_val) *v);
  }
  WABI_SET_TAG(env, wabi_tag_env);
}

void
wabi_store_collect_heap(wabi_store store)
{
  wabi_word size;
  store->scan = store->new_space;
  do {
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
      (store->scan)+=WABI_SYMBOL_SIZE;
      break;

    case wabi_tag_bin_leaf:
      wabi_binary_collect_val(store, (wabi_binary) store->scan);
      break;

    case wabi_tag_pair:
      *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      WABI_SET_TAG((store->scan), wabi_tag_pair);
      (store->scan) += 2;
      break;

    case wabi_tag_map_entry:
      *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      WABI_SET_TAG((store->scan), wabi_tag_map_entry);
      (store->scan) += 2;
      break;

    case wabi_tag_map_array:
      size = *((store->scan) + 1);
      wordcopy(store->heap, (wabi_word*) WABI_WORD_VAL(*(store->scan)), 2 * size);
      *(store->scan) = (wabi_word) store->heap;
      store->heap += 2 * size;
      WABI_SET_TAG((store->scan), wabi_tag_map_array);
      (store->scan) += 2;
      break;

    case wabi_tag_map_hash:
      size = WABI_MAP_BITMAP_COUNT(*((store->scan) + 1));
      wordcopy(store->heap, (wabi_word*) WABI_WORD_VAL(*(store->scan)), 2 * size);
      *(store->scan) = (wabi_word) store->heap;
      store->heap += 2 * size;
      WABI_SET_TAG((store->scan), wabi_tag_map_hash);
      (store->scan) += 2;
      break;

    case wabi_tag_bt_app:
    case wabi_tag_bt_oper:
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      if(*((store->scan) + 2)) {
        *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      }
      (store->scan) += WABI_COMBINER_BUILTIN_SIZE;
      break;


    case wabi_tag_oper:
      *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      *((store->scan) + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 3));
      *((store->scan) + 4) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 4));
      WABI_SET_TAG((store->scan), wabi_tag_oper);
      (store->scan) += WABI_COMBINER_DERIVED_SIZE;
      break;

    case wabi_tag_app:
      *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      *((store->scan) + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 3));
      *((store->scan) + 4) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 4));
      WABI_SET_TAG((store->scan), wabi_tag_app);
      (store->scan) += WABI_COMBINER_DERIVED_SIZE;
      break;

    case wabi_tag_env:
      wabi_store_collect_env(store, (wabi_env) (store->scan));
      (store->scan) += wabi_store_env_size((wabi_env) (store->scan));
      break;

    case wabi_tag_cont_eval:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      WABI_SET_TAG((store->scan), wabi_tag_cont_eval);
      (store->scan) += 1;
      break;

    case wabi_tag_cont_prompt:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      WABI_SET_TAG((store->scan), wabi_tag_cont_eval);
      (store->scan) += 2;
      break;

    case wabi_tag_cont_apply:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      WABI_SET_TAG((store->scan), wabi_tag_cont_apply);
      (store->scan) += 3;
      break;

    case wabi_tag_cont_call:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      WABI_SET_TAG((store->scan), wabi_tag_cont_call);
      (store->scan) += 3;
      break;

    case wabi_tag_cont_def:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      WABI_SET_TAG((store->scan), wabi_tag_cont_def);
      (store->scan) += 3;
      break;

    case wabi_tag_cont_prog:
      // todo: unify all these cases
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      WABI_SET_TAG((store->scan), wabi_tag_cont_prog);
      (store->scan) += 3;
      break;

    case wabi_tag_cont_args:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      *((store->scan) + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 3));
      WABI_SET_TAG((store->scan), wabi_tag_cont_args);
      (store->scan) += 3;
      break;

    case wabi_tag_cont_sel:
      if(WABI_WORD_VAL(*(store->scan))) {
        *(store->scan) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*(store->scan)));
      }
      *((store->scan) + 1) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 1));
      *((store->scan) + 2) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 2));
      *((store->scan) + 3) = (wabi_word) wabi_store_copy_val(store, (wabi_word*) *((store->scan) + 3));
      WABI_SET_TAG((store->scan), wabi_tag_cont_sel);
      (store->scan) += 4;
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
  memset(store->old_space, 0, WABI_WORD_SIZE * store->size);
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
  if (val < store->new_space || val >=store->limit) {
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
