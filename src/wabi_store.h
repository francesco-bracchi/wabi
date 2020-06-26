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
wabi_store_init(const wabi_store store,
                const wabi_word size);

void
wabi_store_destroy(const wabi_store store);


void
wabi_store_prepare(const wabi_store store);


static inline int
wabi_store_free_words(const wabi_store store)
{
  return store->limit - store->heap;
}

static inline void*
wordcopy(wabi_word *dst, const wabi_word *src, const wabi_size size)
{
  return memcpy(dst, src, size * WABI_WORD_SIZE);
}

#endif
