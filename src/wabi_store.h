#ifndef wabi_store_h

#define wabi_store_h

#include <stddef.h>
#include <string.h>
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


static inline wabi_word*
wabi_store_alloc(wabi_store store, wabi_size size)
{
  wabi_word* res;
  if(store->heap + size < store->limit) {
    res = store->heap;
    store->heap += size;
    return res;
  }
  return NULL;
}

static inline int
wabi_store_free_words(wabi_store store)
{
  return store->limit - store->heap;
}

static inline void*
wordcopy(wabi_word *dst, wabi_word *src, wabi_size size)
{
  return memcpy(dst, src, size * WABI_WORD_SIZE);
}

/** debug only **/
int
wabi_store_check(wabi_store store, wabi_val val);

#endif
