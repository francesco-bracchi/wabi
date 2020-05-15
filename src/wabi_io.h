#ifdef wabi_io_h

#define wabi_io_h

#include "wabi_system.h"
#include "wabi_value.h"

typedef struct wabi_io_struct {
  wabi_system sys;
  struct ev_loop *loop;
  pthread_t thread;
} wabi_io_t;


typedef struct wabi_io_t* wabi_io;

int
wabi_io_init(wabi_io io, wabi_system sys);


void
wabi_io_destroy(wabi_io io);


#endif
