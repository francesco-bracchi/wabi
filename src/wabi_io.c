#define wabi_io_c

#include <stdio.h>

#include "ev.h"
#include "wabi_system.h"
#include "wabi_value.h"
#include "wabi_io.h"


/* void* */
/* wabi_io_run(void* args) { */
/*   wabi_io io; */
/*   io = (wabi_io) args; */

/*   ev_run(io->loop, 0); */
/* } */


/* int */
/* wabi_io_init(wabi_io io, wabi_system sys) */
/* { */
/*   io->sys = sys; */
/*   io->loop = wabi_ev_loop(0); */
/*   pthread_create(&io->thread, NULL, &wabi_io_run, io); */
/* } */

/* void */
/* wabi_io_destroy(wabi_io io) */
/* { */
/*   return; */
/* } */

/* void */
/* wabi_io_wait(wabi_io io) */
/* { */
/*   pthread_join(&io->thread); */
/* } */


/* static void */
/* wabi_io_cb(struct ev_loop *loop, ev_io *w, int rev) */
/* { */
/*   wabi_vm vm; */
/*   wabi_val call = (wabi_val) w->data; */
/*   ev_io_stop(w); */
/*   vm = wabi_system_new_vm(); */
/*   wabi_system_run(sys, vm); */
/*   ev_break(loop); */
/*   free(w); */
/* } */


/* void */
/* wabi_io_watch(wabi_io io, fd fd, int events, wabi_val cb) */
/* { */
/*   ev_io* watcher; */
/*   watcher->data = (void*) cb; */

/*   ev_init (&watcher, wabi_io_cb); */
/*   ev_io_set (&watcher, fd, events); */
/*   ev_io_start(io->loop, &watcher); */
/* } */
