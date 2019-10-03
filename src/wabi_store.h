#ifndef wabi_store_h

#define wabi_store_h

#include "wabi_word.h"
#include "wabi_value.h"

typedef struct wabi_store_struct
{
  wabi_word* space;
  wabi_word* limit;
  wabi_word* heap;
  wabi_word* old_space;
  wabi_word size;
  short int low_threshold;
  short int high_threshold;
} wabi_store_t;

typedef wabi_store_t* wabi_store;

int
wabi_store_init(wabi_store store,
                wabi_word size);

void
wabi_store_destroy(wabi_store store);

wabi_word*
wabi_store_collect_prepare(wabi_store store);

wabi_word*
wabi_store_copy_val(wabi_store store, wabi_word *src);

int
wabi_store_collect(wabi_store store);


static inline int
wabi_store_has_rooms(wabi_store store, wabi_size size)
{
  return store->heap + size < store->limit;
}

static inline wabi_word*
wabi_store_alloc(wabi_store store, wabi_size size)
{
  wabi_word* res;
  res = store->heap;
  store->heap += size;
  return res;
}

#endif
