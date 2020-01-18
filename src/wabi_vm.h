#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_error.h"

typedef wabi_val wabi_control;

typedef struct wabi_vm_struct {
  wabi_val control;
  wabi_val env;
  wabi_val continuation;
  wabi_val symbol_table;
  wabi_val prompt_table;
  wabi_val nil;
  wabi_store_t store;
  wabi_size fuel;
  wabi_error_type error;
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

void
wabi_vm_run(wabi_vm vm);

void
wabi_vm_init(wabi_vm vm, wabi_size store_size);

void
wabi_vm_destroy(wabi_vm vm);

int
wabi_vm_prepare(wabi_vm vm, wabi_size size);

int
wabi_vm_collect(wabi_vm vm);

static inline wabi_word*
wabi_vm_alloc(wabi_vm vm, wabi_size size)
{
  wabi_store store;
  store = &(vm->store);
  return wabi_store_alloc(store, size);
}

#endif
