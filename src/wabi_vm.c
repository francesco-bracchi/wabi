#define wabi_vm_c

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_pair.h"
#include "wabi_store.h"

#define SUCCESS 0
#define SUSPEND 1


#define CONT_MAX_SIZE 10000

void
wabi_vm_init(wabi_vm vm, wabi_size_t size)
{
  vm->errno = 0;
  wabi_store_init(wabi_vm_store(vm), size);
  vm->continuation = (wabi_word_t*) malloc(sizeof(wabi_word_t) * CONT_MAX_SIZE);
  vm->top = vm->continuation;
  vm->continuation_limit = vm->continuation + CONT_MAX_SIZE;
}


void
wabi_vm_push_bind(wabi_vm vm, wabi_val symbol, wabi_val rest)
{
  WABI_CONT_BIND_SIZE;
}


void
wabi_vm_free(wabi_vm vm) {
  wabi_store_free(wabi_vm_store(vm));
}


void
wabi_vm_bind(wabi_vm vm,
             wabi_val env,
             wabi_val pattern,
             wabi_val value)
{
  do {
    switch(wabi_val_tag(pattern)) {
    case WABI_TAG_IGNORE:
      return;
    case WABI_TAG_SYMBOL:
      wabi_env_def(vm, (wabi_env) env, (wabi_symbol) pattern, value);
      return;
    case WABI_TAG_PAIR:
      if(wabi_val_is_pair(value)) {
        wabi_vm_bind(vm, env, wabi_car_raw(pattern), wabi_car_raw(value));
        if(vm->errno) return;
        pattern = wabi_cdr_raw(pattern);
        value = wabi_cdr_raw(value);
        break;
      }
    default:
      if(wabi_eq_raw(pattern, value)) {
        return;
      }
      vm->errno = WABI_ERROR_BIND;
      return;
    }
  } while(1);
}

void
wabi_vm_step(wabi_vm vm)
{
  switch(wabi_val_tag(vm->control)) {
  case WABI_TAG_SYMBOL:
    wabi_val val = wabi_env_lookup(vm->env, vm->control);
    if(val) {
      vm->control = val;
      return;
    }
    vm->errno = WABI_ERROR_LOOKUP_FAIL;
    return;
  case WABI_TAG_PAIR:
    vm->control = wabi_car_raw(vm->control);
    wabi_vm_push_cont_apply(vm, wabi_cdr_raw(vm->control));
    return;
  case WABI_TAG_OPERATIVE:
    wabi_combiner_derived combiner = vm->control;
    wabi_apply_cont apply_cont = (wabi_apply_cont) wabi_vm_cont_pop(vm);
    wabi_env next_env = wabi_env_extend(vm, combiner->static_env);
    wabi_env_def(vm, next, (wabi_symbol) combiner->caller_env_name, vm->env);
    wabi_vm_bind(vm, combiner->arguments, apply_cont->arguments);
    if(vm->errno) return;

    vm->control = combiner->body;
    vm->env = next_env;
    return;
  case WABI_TAG_BUILTIN_OP:
    wabi_vm_call(vm->control, wabi_vm_pop(vm));
    return;
  }
}


void
wabi_vm_stop()
{
}
