#define wabi_system_c

#include "wabi_system.h"
#include "wabi_store.h"
#include "wabi_value.h"
#include "wabi_map.h"

int
wabi_system_init(wabi_system sys)
{
  int res;
  res = wabi_store_init(sys->global_store, sys->config->store_initial_size);
  if(!res) return 0;

  sys->nil = (wabi_val) wabi_store_heap_alloc(sys->global_store, 1);
  *(sys->nil) = wabi_val_nil;
  sys->symbol_table = (wabi_map) wabi_map_empty(sys->global_store);
  return 1;
}

void
wabi_system_destroy(wabi_system sys)
{
  wabi_store_destroy(&(sys->global_store));
}
