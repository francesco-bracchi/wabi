#ifndef wabi_store_h

#define wabi_store_h

#include "wabi_value.h"

typedef struct wabi_store
{
  wabi_size_t size;
  wabi_word_t *from_space;
  wabi_word_t *limit;
  wabi_word_t *scan;
  wabi_word_t *root;
  wabi_word_t *alloc;
  wabi_word_t *to_space;
  wabi_word_t *symbol_table;
} wabi_store_t;

typedef wabi_store_t* wabi_store;

int
wabi_store_init(wabi_store store, wabi_size_t size);


void
wabi_store_free(wabi_store store);


int
wabi_store_collect(wabi_store store);


wabi_word_t*
wabi_store_allocate(wabi_store store, wabi_size_t size);


wabi_word_t
wabi_store_used(wabi_store store);


wabi_word_t
wabi_store_total(wabi_store store);

#endif
