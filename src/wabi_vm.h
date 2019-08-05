#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"
#include "wabi_store.h"

typedef struct wabi_vm_struct
{
  wabi_val control;
  wabi_val environment;
  wabi_val continuation;
  wabi_val err_meta;
  wabi_store_t store;
  int errno;
  unsigned int fuel;
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

#define wabi_vm_store(vm) (&(vm->store))

#define wabi_vm_allocate(vm, size) (wabi_store_allocate(wabi_vm_store(vm), size))

#define wabi_vm_collect(vm) (wabi_store_collect(wabi_vm_store(vm)))


void
wabi_vm_init(wabi_vm vm, wabi_size_t size);

void
wabi_vm_free(wabi_vm vm);

void
wabi_vm_bind(wabi_vm vm,
             wabi_val env,
             wabi_val pattern,
             wabi_val value);

#endif
