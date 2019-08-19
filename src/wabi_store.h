#ifndef wabi_store_h

#define wabi_store_h

#include "wabi_word.h"
#include "wabi_value.h"

typedef struct wabi_store_struct
{
  wabi_word* stack;
  wabi_word* heap;

  wabi_word* space;
  wabi_word* limit;
  wabi_word size;
} wabi_store_t;

typedef wabi_store_t* wabi_store;

int
wabi_store_init(wabi_store store,
                wabi_word size);


void
wabi_store_free(wabi_store store);

#endif
