#ifndef wabi_system_h

#define  wabi_system_h

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_queue.h"

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct wabi_system_config_struct
{
  wabi_size store_size;
  wabi_word fuel;
  wabi_word num_threads;
} wabi_system_config_t;

typedef wabi_system_config_t* wabi_system_config;


typedef struct wabi_system_rts_struct
{
  wabi_queue_t vm_queue;
  pthread_t *threads;
  pthread_mutex_t vmlock;
  pthread_cond_t vmcond;
  wabi_size vmcnt;
} wabi_system_rts_t;


typedef struct wabi_system_struct
{
  wabi_system_config_t config;
  wabi_system_rts_t rts;
} wabi_system_t;


typedef wabi_system_t* wabi_system;


void
wabi_system_init(wabi_system sys);

void
wabi_system_destroy(wabi_system sys);

wabi_vm
wabi_system_new_vm();

void
wabi_system_run(wabi_system sys, wabi_vm vm);

void
wabi_system_wait(wabi_system sys);

#endif
