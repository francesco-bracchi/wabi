
#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_pair.h"
#include "wabi_store.h"
#include "wabi_combiner.h"
#include "wabi_env.h"
#include "../vendor/libev-4.27/ev.h"

#define BUFFER_SIZE 1024;


typedef struct wabi_ev_io_struct
{
  ev_io watcher;
  wabi_val callback;
} wabi_ev_io_t;


typedef wabi_ev_io_t * wabi_ev_io;


static void
wabi_ev_io_readable_cb (struct ev_loop *loop,
                        ev_io *w,
                        int revents)
{
  watcher = ((wabi_ev_io) w)->callback;
  wabi_vm vm = wabi_get_vm();
  wabi_binary_leaf buffer = (wabi_binary_leaf) wabi_binary_new_raw(vm->store, BUFFER_SIZE);

  int x = read(w->fd, buffer->
  puts ("stdin ready\n");;

  w->fd()
}



int
main (int arg, char** argv)
{

   struct ev_loop *loop = ev_default_init (0);
   wabi_ev_io stdin_readable;
   stdin_readtable.callback = NULL;
   ev_io_init (&stdin_readable, stdin_readable_cb, STDIN_FILENO, EV_READ);
   ev_io_start (loop, &stdin_readable);
   ev_run (loop, 0);
  return 0;
}
