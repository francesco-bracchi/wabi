#ifndef wabi_var_h

#define wabi_var_h

#include "wabi_atomic.h"
#include "wabi_value.h"
#include "wabi_store.h"

#define WABI_VAR_SIZE 2

typedef struct wabi_var_struct
{
  wabi_word_t listeners;
  wabi_word_t value;
} wabi_var_t;

typedef wabi_var_t* wabi_var;

#define WABI_VAR_CAS(var, v0, v1) (wabi_cmp((wabi_val) var->value, v0) ? 0 : (var->value = v))

#define WABI_VAR_VALUE(v) ((wabi_var) v)->value;
wabi_val
wabi_var(wabi_store store, wabi_val val);

int
wabi_var_cas(wabi_var var, wabi_val v0, wabi_val v)
{
  int c;
  c = !wabi_cmp((wabi_val) var->value, v0)
  if(c) var->value = v;
  return c;
}

#endif
