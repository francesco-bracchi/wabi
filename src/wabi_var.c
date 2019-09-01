#define wabi_var_c

#include "wabi_atomic.h"
#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_var.h"

wabi_val
wabi_var(wabi_store store, wabi_val val) {
  wabi_var var = wabi_store_allocate(store, WABI_VAR_SIZE);
  if(var) {
    var>listeners = 0U & WABI_TAG_VAR;
    var->value = (wabi_word_t) val;
    return var;
  }
  return NULL;
}

int
wabi_var_cas(wabi_var var, wabi_val v0, wabi_val v)
{
  int c;
  c = wabi_cmp((wabi_val) var->value, v0)
  if(!c) var->value = v;
  return c;
}
