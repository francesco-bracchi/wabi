#ifndef wabi_constant_h

#define wabi_constant_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_error.h"
#include "wabi_env.h"

#define WABI_CONSTANT_SIZE 1

void
wabi_constant_builtins(const wabi_vm vm, const wabi_env env);


static inline int
wabi_is_nil(const wabi_val v) {
  return *v == wabi_val_nil;
}

static inline int
wabi_is_ignore(const wabi_val v) {
  return *v == wabi_val_ignore;
}

static inline int
wabi_is_boolean(const wabi_val v) {
  return *v == wabi_val_true || *v == wabi_val_false;
}

static inline int
wabi_is_empty(const wabi_val v) {
  return *v == wabi_val_empty;
}

static inline int
wabi_is_falsey(const wabi_val v) {
  return *v == wabi_val_false || *v == wabi_val_nil;
}

#endif
