#define wabi_builtin_c

#include "wabi_value.h"
#include "wabi_pair.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_store.h"

void
wabi_builtin_sum(wabi_val *control, wabi_env *env, wabi_store *store, wabi_cont *cont)
{
  // todo: handle wrong types
  // todo: handle overlflows
  long res = 0;
  wabi_val ctrl = *control;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    res += WABI_CAST_INT64(wabi_car((wabi_pair) ctrl));
    ctrl = wabi_cdr((wabi_pair) ctrl);
  }
  *control = wabi_fixnum_new(*store, res);
}

void
wabi_builtin_oper(wabi_val *control, wabi_env *env, wabi_store *store, wabi_cont *cont)
{
  wabi_val fs, e, b;

  fs = wabi_car(*control);
  e  = wabi_car(wabi_cdr(*control));
  b  = wabi_car(wabi_cdr(wabi_cdr(*control)));
  if(fs && e && b) {
    *control = wabi_combiner_new(*store, (wabi_env) *env, e, fs, b);
    return 0;
  }
  return 1;
}

void
wabi_builtin_wrap(wabi_val *control, wabi_env *env, wabi_store *store, wabi_cont *cont)
{
  *control = wabi_combiner_wrap(*store, wabi_car(*control));
  return 0;
}


wabi_env
wabi_builtin_stdenv(wabi_store store)
{

}
