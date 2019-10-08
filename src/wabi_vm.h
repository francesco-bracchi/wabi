#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"
#include "wabi_store.h"

typedef wabi_val wabi_control;

typedef enum wabi_vm_result_enum
  {
   wabi_vm_result_done = 0,
   wabi_vm_result_suspended = 1,
   wabi_vm_result_error = 2
  } wabi_vm_result;

typedef struct wabi_vm_struct {
  wabi_val control;
  wabi_val env;
  wabi_val continuation;
  wabi_val tree;
  wabi_store_t store;
  int errno;
  wabi_val errval;
  int fuel;
  // wabi_store shared_store;
  // wabi_map symbol_table;
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

wabi_vm_result
wabi_vm_run(wabi_vm vm);

int
wabi_vm_init(wabi_vm vm, wabi_size store_size);

void
wabi_vm_destroy(wabi_vm vm);

int
wabi_vm_prepare(wabi_vm vm, wabi_size size);

static inline int
wabi_vm_has_rooms(wabi_vm vm, wabi_size size)
{
  wabi_store store;
  store = &(vm->store);
  return wabi_store_has_rooms(store, size);
}

static inline wabi_word*
wabi_vm_alloc(wabi_vm vm, wabi_size size)
{
  wabi_store store;
  store = &(vm->store);
  return wabi_store_alloc(store, size);
}

#endif
