#define wabi_io_c

#include <stdio.h>

#include "ev.h"
#include "wabi_system.h"

int wabi_loop(wabi_system sys)
{
  struct ev_loop *loop;
  wabi_system_t wabi_sys;

  wabi_sys.config.store_initial_size = 10240;
  wabi_system_init(&wabi_sys);

  wabi_system_destroy(&wabi_sys);
}
