/**
 * Memory manager
 */

#define wabi_store_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_err.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_hash.h"
#include "wabi_map.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_secd_frame.h"

#define WABI_STORE_LIMIT (wabi_word_t *)0x00FFFFFFFFFFFFFF

inline static wabi_word_t *
wabi_store_allocate_space(wabi_word_t size)
{
  return (wabi_word_t *) malloc(WABI_WORD_SIZE * size);
}


int
wabi_store_init(wabi_store store, wabi_size_t size)
{
  store->size = size;
  store->from_space = wabi_store_allocate_space(size);

  if(store->from_space == NULL) {
    return WABI_ERROR_NOMEM;
  }
  if(store->from_space + store->size > WABI_STORE_LIMIT) {
    return WABI_ERROR_NOMEM;
  }

  store->limit = store->from_space + store->size;
  store->alloc = store->from_space;
  store->scan = NULL;
  store->symbol_table = (wabi_word_t*) wabi_map_empty_raw(store);
  return WABI_ERROR_NONE;
}

void
wabi_store_free(wabi_store store)
{
  free(store->from_space);
}


inline static void
wabi_store_compact_binary(wabi_store store, wabi_val src)
{
  wabi_size_t len, word_size;

  wabi_binary_leaf_t *new_leaf;
  wabi_val new_blob;
  char *data;

  len = *src & WABI_VALUE_MASK;
  word_size = wabi_binary_word_size(len);
  new_leaf = (wabi_binary_leaf_t *) store->alloc;
  store->alloc += WABI_BINARY_LEAF_SIZE;

  new_blob = store->alloc;
  store->alloc += 2 + word_size;

  *new_blob = len | WABI_TAG_BIN_BLOB;

  data = (char *) (new_blob + 1);

  new_leaf->length = (wabi_word_t) len | WABI_TAG_BIN_LEAF;
  new_leaf->data_ptr = (wabi_word_t) data;

  wabi_binary_compact_raw(src, data);
}


wabi_word_t *
wabi_store_copy_val(wabi_store store, wabi_word_t *src)
{
  wabi_word_t tag = wabi_val_tag(src);
  if(tag == WABI_TAG_FORWARD) {
    return (wabi_word_t *) (*src & WABI_VALUE_MASK);
  }
  wabi_word_t* res = store->alloc;

  if(tag <= WABI_TAG_ATOMIC_LIMIT || tag == WABI_TAG_SYMBOL) {
    *res = *src;
    store->alloc++;
  } else switch(tag) {
    case WABI_TAG_PAIR:
      memcpy(res, src, 2 * WABI_WORD_SIZE);
      store->alloc+=2;
      break;
    case WABI_TAG_MAP_ARRAY:
    case WABI_TAG_MAP_HASH:
    case WABI_TAG_MAP_ENTRY:
      memcpy(res, src, WABI_MAP_BYTE_SIZE);
      store->alloc += WABI_MAP_SIZE;
      break;
    case WABI_TAG_BIN_LEAF:
    case WABI_TAG_BIN_NODE:
      wabi_store_compact_binary(store, src);
      break;
    case WABI_TAG_ENV:
      memcpy(res, src, WABI_ENV_BYTE_SIZE);
      store->alloc+= WABI_ENV_SIZE;
      break;
    case WABI_TAG_SECD_FRAME:
      memcpy(res, src, WABI_FRAME_BYTE_SIZE);
      store->alloc+= WABI_ENV_SIZE;
      break;
    default:
      return NULL;
    }
  *src = WABI_TAG_FORWARD | ((wabi_word_t) res);
  return res;
}


inline static void
wabi_store_collect_pair(wabi_store store, wabi_pair pair)
{
  pair->car = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) (pair->car | WABI_VALUE_MASK)) | WABI_TAG_PAIR;
  pair->cdr = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) pair->cdr);
  store->scan += 2;
}


inline static void
wabi_store_collect_symbol(wabi_store store, wabi_val sym)
{
  wabi_val bin = (wabi_val) (*sym & WABI_VALUE_MASK);
  wabi_val interned = wabi_map_get_raw((wabi_map) store->symbol_table, bin);
  if(interned != NULL) {
    wabi_val new_bin = wabi_store_copy_val(store, bin);
    *sym = (wabi_word_t) new_bin | WABI_TAG_SYMBOL;
    store->symbol_table = (wabi_word_t*) wabi_map_assoc_raw(store, (wabi_map) store->symbol_table, new_bin, sym);
  }
  store->scan++;
}

inline static void
wabi_store_collect_secd_frame(wabi_store store, wabi_secd_frame frame)
{
  frame->stack = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) frame->stack & WABI_VALUE_MASK) & WABI_TAG_SECD_FRAME;
  frame->environment = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) frame->frame);
  frame->control = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) frame->control);
  store->scan+=3;
}


inline static void
wabi_store_collect_map_entry(wabi_store store,
                             wabi_map_entry map)
{
  wabi_val key = wabi_store_copy_val(store, WABI_MAP_ENTRY_KEY(map));
  wabi_val value = wabi_store_copy_val(store, WABI_MAP_ENTRY_VALUE(map));
  map->key = (wabi_word_t) key;
  map->value = (wabi_word_t) value | WABI_TAG_MAP_ENTRY;
  store->scan += 2;
}


inline static void
wabi_store_collect_map_array(wabi_store store,
                           wabi_map_array map)
{
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  if(size) {
    // todo: remove the if or not? the output is the same, number of operations not.
    wabi_map table = WABI_MAP_ARRAY_TABLE(map);
    wabi_map res = (wabi_map) store->alloc;

    memcpy(res, table, WABI_MAP_BYTE_SIZE * size);
    store->alloc += size * WABI_MAP_SIZE;
    map->table = (wabi_word_t) res | WABI_TAG_MAP_ARRAY;
  }
  store->scan += WABI_MAP_SIZE;
}


inline static void
wabi_store_collect_map_hash(wabi_store store,
                          wabi_map_hash map)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_map table = WABI_MAP_HASH_TABLE(map);
  wabi_word_t* res = store->alloc;

  memcpy(res, table, WABI_MAP_BYTE_SIZE * size);
  store->alloc += WABI_MAP_SIZE * size;
  map->table = (wabi_word_t) res | WABI_TAG_MAP_HASH;
  store->scan += WABI_MAP_SIZE;
}


inline static void
wabi_store_collect_environmnet(wabi_store store, wabi_env env)
{
  wabi_env prev;
  env->data = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) env->data);
  prev = (wabi_env) (env->prev & WABI_VALUE_MASK);
  if(prev) {
    env->prev = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) prev) | WABI_TAG_ENV;
  }

  store->scan += WABI_ENV_SIZE;
}


inline static void
wabi_store_collect_combiner_derived(wabi_store store,
                                    wabi_combiner_derived combiner)
{
  wabi_word_t tag = combiner->static_env & WABI_TAG_MASK;
  combiner->static_env = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) (combiner->static_env | WABI_VALUE_MASK)) | tag;
  combiner->caller_env_name = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) (combiner->caller_env_name | WABI_VALUE_MASK));
  combiner->arguments = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) (combiner->arguments | WABI_VALUE_MASK));
  combiner->body = (wabi_word_t) wabi_store_copy_val(store, (wabi_val) (combiner->body | WABI_VALUE_MASK));

  store->scan+= WABI_COMBINER_DERIVED_SIZE;
}


inline static int
wabi_store_collect_all(wabi_store store)
{
  while(store->scan < store->alloc) {

    wabi_word_t tag;
    tag = wabi_val_tag(store->scan);

    if(tag <= WABI_TAG_ATOMIC_LIMIT) {
      store->scan++;
    }
    else {
      switch(tag) {
      case WABI_TAG_PAIR:
        wabi_store_collect_pair(store, (wabi_pair) store->scan);
        break;
      case WABI_TAG_BIN_BLOB:
        store->scan += (*store->scan & WABI_VALUE_MASK);
        break;
      case WABI_TAG_BIN_LEAF:
        store->scan += 3;
        break;
      case WABI_TAG_SYMBOL:
        wabi_store_collect_symbol(store, store->scan);
        break;
      case WABI_TAG_MAP_ENTRY:
        wabi_store_collect_map_entry(store, (wabi_map_entry) store->scan);
        break;
      case WABI_TAG_MAP_ARRAY:
        wabi_store_collect_map_array(store, (wabi_map_array) store->scan);
        break;
      case WABI_TAG_MAP_HASH:
        wabi_store_collect_map_hash(store, (wabi_map_hash) store->scan);
        break;
      case WABI_TAG_ENV:
        wabi_store_collect_environmnet(store, (wabi_env) store->scan);
        break;
      case WABI_TAG_BUILTIN_OP:
      case WABI_TAG_BUILTIN_APP:
        // todo: find out if it would be better to have a check on the builtin bit (i.e. if)
        store->scan++;
        break;
      case WABI_TAG_OPERATIVE:
      case WABI_TAG_APPLICATIVE:
        // todo: find out if it would be better to have a check on the derived bit (i.e. if)
        wabi_store_collect_combiner_derived(store, (wabi_combiner_derived) store->scan);
        break;
      case WABI_TAG_SECD_FRAME:
        wabi_store_collect_secd_frame(store, (wabi_frame) store->scan);
        break;
      default:
        return WABI_ERROR_UNKNOWN;
      }
    }
  }
  return WABI_ERROR_NONE;
}


int
wabi_store_collect(wabi_store store)
{
  int result;

  store->to_space = store->from_space;
  store->from_space = wabi_store_allocate_space(store->size);
  store->symbol_table = (wabi_word_t *) wabi_map_empty_raw(store);

  if(store->from_space == NULL) {
    return WABI_ERROR_NOMEM;
  }
  if(store->from_space + store->size > WABI_STORE_LIMIT) {
    return WABI_ERROR_NOMEM;
  }
  store->limit = store->from_space + store->size;
  store->alloc = store->from_space;
  store->scan = store->from_space;

  store->root = wabi_store_copy_val(store, store->root);
  result = wabi_store_collect_all(store);
  free(store->to_space);

  return result;
}


wabi_word_t*
wabi_store_allocate(wabi_store store, wabi_size_t size)
{
  /* if(store->alloc + size >= store->limit) */
  /*   wabi_collect(store); */

  if(store->alloc + size >= store->limit) {
    return NULL;
  }
  wabi_word_t* res = store->alloc;
  store->alloc += size;
  return res;
}


wabi_word_t
wabi_store_used(wabi_store store)
{
  // if not running gc!
  return (wabi_word_t)(store->alloc - store->from_space);
}


wabi_word_t
wabi_store_total(wabi_store store)
{
  // if not running gc!
  return (wabi_word_t)(store->limit - store->from_space);
}
