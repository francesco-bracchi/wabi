#ifndef wabi_vm_h

#define wabi_vm_h


#include "wabi_value.h"
#include "wabi_system.h"
#include "wabi_env.h"
#include "wabi_cont.h"

typedef wabi_val wabi_control;

typedef enum wabi_vm_result_enum
  {
   wabi_vm_result_done = 0,
   wabi_vm_result_suspended = 1,
   wabi_vm_result_error = 2
  } wabi_vm_result;

// rename to state?
typedef struct wabi_vm_struct {
  wabi_val control;
  wabi_env env;
  wabi_cont continuation;
  wabi_store store;
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
wabi_vm_init(wabi_vm vm);

void
wabi_vm_destroy(wabi_vm vm);

#endif
