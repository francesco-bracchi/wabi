#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_error.h"

#include <stdio.h>

typedef wabi_val wabi_control;

typedef struct wabi_vm_struct {
  /** control register, where the the object of the action lies **/
  wabi_val        ctrl;

  /** environment register, the vocabulary (i.e. the Code) that should be used to execute the command **/
  wabi_val        env;

  /** Continuation register it's similar to the stack. Can comprise different action types **/
  wabi_val        cont;

  /** Another register that references the latest prompt, used to accelerate the `control` operator **/
  wabi_val        prmt;

  /** Symbol table: used to avoid repetitions in symbols **/
  wabi_val        stbl;

  /** Since is used everywere it's worth to have here **/
  wabi_val        nil;

  /** Values (i.e. symbols) we want in any case **/
  wabi_val        oth;

  /** Store is the heap where VM values lie **/
  wabi_store_t    stor;

  /** Number of reductions the VM has to do before interrupting **/
  wabi_size       fuel;

  /** Error value, holds extra information in case of error **/
  wabi_val        erv;

  /** Error type see `wabi_errors.h` **/
  wabi_error_type ert;
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

void
wabi_vm_run(wabi_vm vm, wabi_word fuel);

void
wabi_vm_init(const wabi_vm vm, const wabi_size size);

void
wabi_vm_destroy(wabi_vm vm);

int
wabi_vm_prepare(wabi_vm vm, wabi_size size);

void
wabi_vm_collect(const wabi_vm vm);

static inline wabi_word*
wabi_vm_alloc(wabi_vm vm, wabi_size size)
{
  wabi_word* res;

  if(vm->stor.heap + size < vm->stor.limit) {
    res = vm->stor.heap;
    vm->stor.heap += size;
    return res;
  }
  vm->ert = wabi_error_nomem;
  return NULL;
}

#endif
