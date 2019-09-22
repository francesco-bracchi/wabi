#define wabi_system_c

#include "wabi_system.h"
#include "wabi_store.h"
#include "wabi_value.h"
#include "wabi_map.h"

int
wabi_system_init(wabi_system sys)
{
  int res;
  /* res = wabi_store_init(sys->global_store, sys->config->store_initial_size); */
  /* if(!res) return 0; */

  /* sys->nil = (wabi_val) wabi_store_alloc(sys->global_store, 1); */
  /* if(! sys->nil) return 0; */

  /* *(sys->nil) = wabi_val_nil; */
  return 1;
}

void
wabi_system_destroy(wabi_system sys)
{
  wabi_store_destroy(sys->global_store);
}
