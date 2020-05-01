#define wabi_system_c

#include "wabi_system.h"
#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_queue.h"

#include <stdio.h>

static inline void
wabi_system_error_signal(wabi_vm vm)
{
  printf("ERROR IN A VM: %s\n", wabi_error_name(vm->error));
}


void *wabi_system_consume_queue(void* args) {
  for(;;) {
    wabi_vm vm = wabi_queue_deq(&wabi_sys.vm_queue);
    wabi_vm_run(vm, wabi_sys.config.fuel);
    switch(vm->error) {
    case wabi_error_timeout:
      wabi_queue_enq(&wabi_sys.vm_queue, vm);
      break;
    case wabi_error_none:
      break;
    default:
      wabi_system_error_signal(vm);
    }
  }
}


void
wabi_system_init(wabi_system_config config)
{
  wabi_word t;

  wabi_sys.config = *config;
  wabi_queue_init(&(wabi_sys.vm_queue));
  wabi_sys.threads = (pthread_t*) malloc(config->num_threads * sizeof(pthread_t));
  for(t = 0; t < config->num_threads; t++) {
    pthread_create(wabi_sys.threads + t, NULL, &wabi_system_consume_queue, NULL);
  }
}


void
wabi_system_destroy()
{
  // todo: wait until the queue is empty and all the threads are waiting for condvar
  wabi_word t;

  wabi_queue_destroy(&(wabi_sys.vm_queue));

  for(t = 0; t < wabi_sys.config.num_threads; t++)
    pthread_cancel(*(wabi_sys.threads + t));
  free(wabi_sys.threads);
}

wabi_vm
wabi_system_new_vm()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, wabi_sys.config.store_size);
  return vm;
}


void
wabi_system_run(wabi_vm vm) {
  printf("wabi_system_run\n");
  wabi_queue_enq(&wabi_sys.vm_queue, vm);
}
