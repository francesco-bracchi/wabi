#define wabi_combiner_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_combiner.h"
#include "wabi_env.h"
#include "wabi_value.h"

wabi_combiner
wabi_combiner_builtin_new(wabi_store store,
                          wabi_binary cname,
                          wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res = (wabi_combiner_builtin) wabi_store_heap_alloc(store, WABI_COMBINER_BUILTIN_SIZE);
  if(res) {
    res->c_ptr = (wabi_word) cfun;
    res->c_name = (wabi_word) cname;
    WABI_SET_TAG(res, wabi_tag_bt_oper);
    return (wabi_combiner) res;
  }
  return NULL;
}

wabi_combiner
wabi_combiner_new(wabi_store store,
                  wabi_env static_env,
                  wabi_val dynamic_env_name,
                  wabi_val parameters,
                  wabi_val body)
{
  wabi_combiner_derived res = (wabi_combiner_derived) wabi_store_heap_alloc(store, WABI_COMBINER_DERIVED_SIZE);
  if(res) {
    res->static_env = (wabi_word) static_env;
    res->caller_env_name = (wabi_word) dynamic_env_name;
    res->parameters = (wabi_word) parameters;
    res->body = (wabi_word) body;
    WABI_SET_TAG(res, wabi_tag_oper);
    return (wabi_combiner) res;
  }
  return NULL;
}

wabi_combiner
wabi_combiner_wrap(wabi_store store, wabi_combiner combiner)
{
  wabi_combiner res;
  switch(WABI_TAG(combiner)) {
  case wabi_tag_oper:
    res = (wabi_combiner) wabi_store_heap_alloc(store, WABI_COMBINER_DERIVED_SIZE);
    memcpy(res, combiner, 8 * WABI_COMBINER_DERIVED_SIZE);
    WABI_SET_TAG(res, wabi_tag_app);
    return res;
  case wabi_tag_bt_oper:
    res = (wabi_combiner) wabi_store_heap_alloc(store, WABI_COMBINER_BUILTIN_SIZE);
    *res = *combiner;
    WABI_SET_TAG(res, wabi_tag_app);
    return res;
  case wabi_tag_bt_app:
  case wabi_tag_app:
    return (wabi_combiner) combiner;
  }
  return NULL;
}

wabi_combiner
wabi_combiner_unwrap(wabi_store store, wabi_combiner combiner)
{
  wabi_combiner res;
  switch(WABI_TAG(combiner)) {
  case wabi_tag_app:
    res = (wabi_combiner) wabi_store_heap_alloc(store, WABI_COMBINER_DERIVED_SIZE);
    memcpy(res, combiner, 8 * WABI_COMBINER_DERIVED_SIZE);
    WABI_SET_TAG(res, wabi_tag_oper);
    return res;
  case wabi_tag_bt_app:
    res = (wabi_combiner) wabi_store_heap_alloc(store, WABI_COMBINER_BUILTIN_SIZE);
    *res = *combiner;
    WABI_SET_TAG(res, wabi_tag_bt_oper);
    return res;
  case wabi_tag_bt_oper:
  case wabi_tag_oper:
    return combiner;
  }
  return NULL;
}
