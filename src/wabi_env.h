#ifndef wabi_env_h

#define wabi_env_h

#include "wabi_map.h"
#include "wabi_store.h"

typedef struct wabi_env_struct {
  wabi_word_t prev;
  wabi_word_t data;
} wabi_env_t;

typedef wabi_env_t* wabi_env;

#define WABI_ENV_SIZE 2


#endif
