#ifndef wabi_vm_h

#define wabi_vm_h

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_error.h"

#include <stdio.h>

typedef wabi_val wabi_control;

typedef struct wabi_vm_cache_item_struct {
  wabi_val sym;
  wabi_val env;
  wabi_val val;
} wabi_vm_cache_item_t;

#define WABI_VM_CACHE_SIZE 8

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

  /** Atom table: used to avoid repetitions in atoms **/
  wabi_val        atbl;

  /** Since is used everywere it's worth to have here **/
  wabi_val        emp;

  /** Since is used everywere it's worth to have here **/
  wabi_val        nil;

  /** Since is used everywere it's worth to have here **/
  wabi_val        ign;

  /** Since is used everywere it's worth to have here **/
  wabi_val        fls;

  /** Since is used everywere it's worth to have here **/
  wabi_val        trh;

  /** Store is the heap where VM values lie **/
  wabi_store_t    stor;

  /** Number of reductions the VM has to do before interrupting **/
  wabi_size       fuel;

  /** Error value, holds extra information in case of error **/
  wabi_val        erv;

  /** Error type see `wabi_errors.h` **/
  wabi_error_type ert;

  /** Env cache **/
  wabi_vm_cache_item_t cache[WABI_VM_CACHE_SIZE];
} wabi_vm_t;

typedef wabi_vm_t* wabi_vm;

void
wabi_vm_run(const wabi_vm vm, const wabi_word fuel);

void
wabi_vm_init(const wabi_vm vm, const wabi_size size);

void
wabi_vm_destroy(const wabi_vm vm);

int
wabi_vm_prepare(const wabi_vm vm, const wabi_size size);

void
wabi_vm_collect(const wabi_vm vm);

static inline wabi_word*
wabi_vm_alloc(const wabi_vm vm, const wabi_size size)
{
  wabi_word* res;

  if (vm->stor.heap + size >= vm->stor.limit) {
    vm->ert = wabi_error_nomem;
    return NULL;
  }
  res = vm->stor.heap;
  vm->stor.heap += size;
  return res;
}

#endif
