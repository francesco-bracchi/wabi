#define wabi_store_c

#include <stdlib.h>
#include "wabi_store.h"
#include "wabi_word.h"

static const wabi_word* wabi_store_limit = (wabi_word *)0x07FFFFFFFFFFFFFF;


int
wabi_store_init(wabi_store store,
                wabi_size size)
{
  wabi_word *mem = malloc(WABI_WORD_SIZE * size);
  if(mem && (mem + size <= wabi_store_limit)) {
    store->space = mem;
    store->limit = mem + size;
    store->heap = store->space;
    store->stack = store->limit;
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

static inline void
wabi_store_collect(wabi_store store)
{
  return;
}

static inline int
wabi_store_has_rooms(wabi_store store, wabi_size size)
{
  if(store->heap + size > store->stack) {
    wabi_store_collect(store);
    return store->heap + size < store->stack;
  }
  return 1;
}


wabi_word *
wabi_store_heap_alloc(wabi_store store, wabi_size size)
{
  wabi_word *res;
  if(wabi_store_has_rooms(store, size)) {
    res = store->heap;
    store->heap += size;
    return res;
  }
  return NULL;
}


wabi_word *
wabi_store_stack_alloc(wabi_store store, wabi_size size)
{
  wabi_word *res;
  if(wabi_store_has_rooms(store, size)) {
    store->stack -= size;
    return store->stack;
  }
  return NULL;
}
