#ifndef wabi_system_h

#define  wabi_system_h

#include "wabi_word.h"
#include "wabi_store.h"
#include <stdint.h>

typedef struct wabi_system_config_struct
{
  uint64_t store_initial_size;
  uint64_t fuel;
  uint64_t stack_size;
  int num_threads;
} wabi_system_config_t;

typedef wabi_system_config_t* wabi_system_config;

typedef struct wabi_system_struct
{
  wabi_system_config_t config;
  wabi_store_t global_store;
} wabi_system_t;

typedef wabi_system_t* wabi_system;

int
wabi_system_init(wabi_system sys);

void
wabi_system_destroy(wabi_system sys);

#endif
