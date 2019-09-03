#define wabi_system_c

#include "wabi_system.h"
#include "wabi_store.h"

int
wabi_system_init(wabi_system sys)
{
  int res;
  res = wabi_store_init(&(sys->global_store), sys->config.store_initial_size);
  if(!res) return 0;
  return 1;
}

void
wabi_system_free(wabi_system sys)
{
  wabi_store_destroy(&(sys->global_store));
}
