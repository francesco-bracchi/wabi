/**
 * Builds a wabi system.
 * a process can have just one wabi system (aka `wabi_sys`
 *
 * TODO:
 * VMs Pooling: min_vm_pool_size, max_vm_pool_size
 */
#define wabi_system_c

#include "wabi_system.h"
#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_queue.h"

#include <stdio.h>


static inline void
wabi_system_error_signal(wabi_vm vm)
{
  printf("ERROR IN A VM: %s\n", wabi_error_name(vm->ert));
}


static inline void
wabi_system_inc_vmc(wabi_system sys) {
  pthread_mutex_lock(&sys->rts.vmlock);
  sys->rts.vmcnt++;
  pthread_mutex_unlock(&sys->rts.vmlock);
}


static inline void
wabi_system_dec_vmc(wabi_system sys) {
  pthread_mutex_lock(&sys->rts.vmlock);
  sys->rts.vmcnt--;
  if(sys->rts.vmcnt <= 0)
    pthread_cond_signal(&sys->rts.vmcond);
  pthread_mutex_unlock(&sys->rts.vmlock);
}


void*
wabi_system_consume_queue(void* args) {
  wabi_system sys;
  sys = (wabi_system) args;

  for(;;) {
    wabi_vm vm = wabi_queue_deq(&sys->rts.vm_queue);
    wabi_vm_run(vm, sys->config.fuel);
    switch(vm->ert) {
    case wabi_error_timeout:
      wabi_queue_enq(&sys->rts.vm_queue, vm);
      break;
    case wabi_error_none:
      wabi_vm_destroy(vm);
      wabi_system_dec_vmc(sys);
      free(vm);
      break;
    default:
      wabi_system_error_signal(vm);
    }
  }
}


void
wabi_system_init(wabi_system sys)
{
  wabi_word t;

  wabi_queue_init(&sys->rts.vm_queue);
  pthread_mutex_init(&sys->rts.vmlock, NULL);
  pthread_cond_init(&sys->rts.vmcond, NULL);
  sys->rts.vmcnt = 0;

  sys->rts.threads = (pthread_t*) malloc(sys->config.num_threads * sizeof(pthread_t));
  for(t = 0; t < sys->config.num_threads; t++) {
    pthread_create(sys->rts.threads + t, NULL, &wabi_system_consume_queue, sys);
  }
}


void
wabi_system_destroy(wabi_system sys)
{
  wabi_word t;
  wabi_system_wait(sys);

  wabi_queue_destroy(&sys->rts.vm_queue);

  pthread_mutex_destroy(&sys->rts.vmlock);
  pthread_cond_destroy(&sys->rts.vmcond);

  for(t = 0; t < sys->config.num_threads; t++)
    pthread_cancel(*(sys->rts.threads + t));
  free(sys->rts.threads);
}

wabi_vm
wabi_system_new_vm(wabi_system sys)
{
  wabi_vm vm;
  vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  if(wabi_vm_init(vm, sys->config.store_size))
    return NULL;
  return vm;
}


void
wabi_system_run(wabi_system sys, wabi_vm vm) {
  wabi_system_inc_vmc(sys);
  wabi_queue_enq(&sys->rts.vm_queue, vm);
}


void
wabi_system_wait(wabi_system sys)
{
  pthread_mutex_lock(&sys->rts.vmlock);
  while(sys->rts.vmcnt > 0)
    pthread_cond_wait(&sys->rts.vmcond, &sys->rts.vmlock);
  pthread_mutex_unlock(&sys->rts.vmlock);
}
