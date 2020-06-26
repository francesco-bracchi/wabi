#define wabi_store_c

#include <stdlib.h>

#include <math.h>
#include "wabi_store.h"

/////////// HERE ///////////////

static const wabi_word* wabi_store_limit = (wabi_word *)0x00FFFFFFFFFFFFFF;


int
wabi_store_init(const wabi_store store,
                const wabi_size size)
{
  wabi_word *new_space = malloc(size * WABI_WORD_SIZE);
  wabi_word *old_space = malloc(size * WABI_WORD_SIZE);
  if(! new_space) return 1;
  if(! old_space) return 2;
  if(new_space + size >= wabi_store_limit) return 3;
  if(old_space + size >= wabi_store_limit) return 4;

  memset(new_space, 0, size * WABI_WORD_SIZE);
  memset(old_space, 0, size * WABI_WORD_SIZE);
  store->new_space = new_space;
  store->limit = new_space + size;
  store->heap = new_space;
  store->size = size;
  store->old_space = old_space;
  return 0;
}


void
wabi_store_destroy(const wabi_store store)
{
  if(store->new_space) free(store->new_space);
  if(store->old_space) free(store->old_space);
}


static inline wabi_word
wabi_store_used(const wabi_store store)
{
  return (store->heap - store->new_space);
}


static inline
wabi_word
wabi_store_free(const wabi_store store)
{
 return (store->limit - store->heap);
}


static inline double
wabi_store_used_ratio(const wabi_store store)
{
  return (double) wabi_store_used(store) / store->size;
}


void
wabi_store_prepare(const wabi_store store)
{
  wabi_word *swap;
  swap = store->new_space;
  store->new_space = store->old_space;
  store->old_space = swap;

  store->heap = store->new_space;
  store->limit = store->new_space + store->size;
  store->scan = store->new_space;
}
