
#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_pair.h"
#include "wabi_store.h"
#include "wabi_combiner.h"

int
wabi_secd_init(wabi_secd vm) {
  wabi_store store = (wabi_store) malloc(sizeof(wabi_store));
  if(store && wabi_store_init(store, WABI_SECD_STORE_SIZE) == WABI_ERROR_NOMEM) {
    vm->nil = wabi_nil_raw(store);
    vm->stack = vm->nil;
    vm->ret = vm_>nil
    vm->environment = NULL;
    vm->control = vm->nil;
    vm->dump = vm->nil;
    vm->vocabulary = wabi_map_empty_raw(vm->store);
  }
  vm->errno = WABI_ERROR_NOMEM;
}


void
wabi_secd_ldc(wabi_vm vm)
{
  wabi_word_t c, s;
  vm->control = WABI_CDR_RAW((wabi_val) vm->control);
  c = WABI_CAR_RAW(vm->control);
  vm->control = WABI_CDR_RAW((wabi_val) vm->control);
  s = wabi_cons_raw((wabi_val) vm->store, c, (wabi_val) vm->stack);
  if(s) {
    vm->stack = s;
    return;
  }
  vm->errno = WABI_VM_NOMEM;
}

void
wabi_secd_lde(wabi_vm vm)
{
  wabi_word_t s;
  wabi_val val, stack;
  s = WABI_CAR_RAW(vm->control);
  vm->control = WABI_CDR_RAW((wabi_val) vm->control);
  val = wabi_env_lookup((wabi_env) vm->environment, (wabi_symbol) s);
  if(val) {
    stack = wabi_cons_raw(vm->store, val, vm->stack);
    if(stack)  {
      vm->stack =  stack;
      vm->control = wabi_cdr_raw(vm->control);
      return;
    }
  }
  vm->error = WABI_ERROR_ENV_LOOKUP;
}























static inline void
wabi_secd_stack_push(wabi_vm vm, wabi_val val)
{
  vm->stack = wabi_cons_raw(val, vm->stack);
}

static inline wabi_val
wabi_secd_stack_pop(wabi_vm vm)
{
  wabi_val res = wabi_car_raw(vm->stack);
  vm->stack = wabi_cdr_raw(vm->stack);
  return res;
}

static inline void
wabi_secd_env_push(wabi_vm vm, wabi_val env)
{
  vm->environment = wabi_cons_raw(env, vm->environment);
}

static inline void
wabi_secd_control_push(wabi_vm vm, wabi_val val)
{
  vm->control = wabi_cons_raw(val, vm->control);
}

static inline void
wabi_secd_dump_push(wabi_vm vm)
{
  wabi_frame frame = (wabi_frame) wabi_store_allocate(vm->store, WABI_SECD_FRAME_SIZE);
  if(frame) {
    memcpy(frame, vm, WABI_SECD_FRAME_BYTE_SIZE);
    frame->stack &= WABI_TAG_FRAME;
    vm->dump = wabi_cons_raw(frame, vm->dump);
    return;
  }
  vm->errno = WABI_ERROR_NOMEM;
}

static inline void
wabi_secd_dump_pop(wabi_vm vm)
{
  wabi_frame frame = wabi_car_raw(vm->dump);
  vm->dump = wabi_cdr_raw(vm->dump);
  memcpy(vm,frame, WABI_SECD_FRAME_BYTE_SIZE);
  vm->stack = vm->stack & WABI_TAG_VALUE;
}

void
wabi_secd_nil(wabi_vm vm)
{
  wabi_secd_stack_push(vm, vm->control);
}

void
wabi_secd_ldc(wabi_vm vm)
{
  vm->control = wabi_cdr_raw(vm->control);
  wabi_secd_stack_push(vm, wabi_car_raw(vm->control));
  vm->control = wabi_cdr_raw(vm->control);
}

void
wabi_secd_ld(wabi_vm vm)
{
  vm->control = wabi_cdr_raw(vm->control);
  wabi_val val = wabi_env_lookup(vm->environment, (wabi_symbol) wabi_car_raw(vm->control));
  if(val) {
    wabi_secd_stack_push(vm, val);
    vm->control = wabi_cdr_raw(vm->control);
    return;
  }
  vm->error = WABI_ERROR_ENV_LOOKUP;
}

void
wabi_secd_if(wabi_vm vm)
{
  wabi_val test = wabi_secd_stack_pop(vm);
  if(!trueish(test)) {
    do {
      test = wabi_secd_control_pop(vm);
    } while (test == &wabi_secd_else);
  }
}

void
wabi_secd_else(wabi_vm vm)
{
  wabi_val test;
  do {
    test = wabi_secd_control_pop(vm);
  } while (test == &wabi_secd_then);
}

void
wabi_secd_then(wabi_vm vm)
{
  wabi_secd_control_pop(vm);
}

void
wabi_secd_dum(wabi_vm vm)
{
  wabi_val env0 = (wabi_val) wabi_env_new_raw(vm->store, vm->environment);
  wabi_secd_env_push(vm, env0);
}

void
wabi_secd_eval(wabi_secd vm, wabi_val expr, wabi_env env) {
  wabi_secd_control_push(vm, wabi_intern(vm, "ev"));
  wabi_secd_control_push(vm, env);
  wabi_secd_control_push(vm, wabi_intern(vm, "lc"));
  wabi_secd_control_push(vm, expr);
  wabi_secd_control_push(vm, wabi_intern(vm, "lc"));

  wabi_secd_control_set(["lc", expr, "lc" env, "e/2"]);
}


int
wabi_secd_step(wabi_secd vm) {
  if(wabi_val_is_nil(vm->control)) {
    return WABI_STATUS_SUCCESS;
  }
  word = wabi_car_raw(vm->control);
  vm->control = wabi_cdr_raw(vm->control);
  action(vm, vm->stack, vm->environment);
}


wabi_val
wabi_secd_pop(wabi_secd vm)
{
  wabi_val expr = wabi_car_raw(vm->stack);
  vm->stack = wabi_cdr_raw(vm->stack);
  return expr;
}


void
wabi_secd_push(wabi_secd vm, wabi_val val)
{
  vm->stack = wabi_cons_raw(vm->store, val, vm->stack);
}


void
wabi_eval(wabi_vm vm, wabi_val stack, wabi_environment env)
{
  expr = wabi_secd_pop(vm);
  switch(wabi_val_tag(expr)) {
  case WABI_TAG_SYMBOL:
    wabi_val v = wabi_env_lookup(env, expr);
    wabi_secd_push(vm, v);
    return;
  case WABI_TAG_PAIR:
    vm_secd_push(wabi_cdr_raw(expr));
    vm_secd_push(wabi_car_raw(expr));
    vm->control = wabi_cons_raw(vm->store, wabi_apply, vm->control);
    vm->control = wabi_cons_raw(vm->store, wabi_eval, vm->control);
    return;
  default:
  }
}
