#ifndef wabi_store_h

#define wabi_store_h

#include <stddef.h>
#include <string.h>
#include "wabi_word.h"
#include "wabi_value.h"

typedef struct wabi_store_struct
{
  wabi_word* new_space;
  wabi_word* limit;
  wabi_word* heap;
  wabi_word* scan;
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

void
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



static inline void
wabi_store_copy_val_size(wabi_store store, wabi_val obj, wabi_size size)
{
  wordcopy(store->heap, obj, size);
  store->heap += size;
}


static inline void
wabi_store_collect_val_size(wabi_store store, wabi_val obj, wabi_size size)
{
  wabi_size cnt;
  wabi_word* pos;
  wabi_word tag;

  // reverse order, because the first word contains references to a kind of tail
  // like pairs, the tail part is the first, byte, the same for continuations.
  tag = WABI_WORD_TAG(*obj);
  cnt = 0;
  pos = obj + (size - 1);
  while(cnt < size) {
    *pos = (wabi_word) wabi_store_copy_val(store, (wabi_word*) WABI_WORD_VAL(*pos));
    cnt++;
    pos--;
  }
  WABI_SET_TAG(obj, tag);
  store->scan+= size;
}

#endif
