#define wabi_combiner_c

#include <stddef.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_combiner.h"
#include "wabi_env.h"
#include "wabi_value.h"
#include "wabi_vm.h"


wabi_combiner
wabi_operator_builtin_new(wabi_vm vm,
                          wabi_binary cname,
                          wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  WABI_SET_TAG(res, wabi_tag_bt_oper);
  return (wabi_combiner) res;
}


wabi_combiner
wabi_application_builtin_new(wabi_vm vm,
                             wabi_binary cname,
                             wabi_builtin_fun cfun)
{
  // todo: verify cfun pointer is less then 2^59
  wabi_combiner_builtin res;
  res = (wabi_combiner_builtin) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);

  res->c_ptr = (wabi_word) cfun;
  res->c_name = (wabi_word) cname;
  WABI_SET_TAG(res, wabi_tag_bt_app);
  return (wabi_combiner) res;
}


wabi_combiner
wabi_combiner_new(wabi_vm vm,
                  wabi_env static_env,
                  wabi_val dynamic_env_name,
                  wabi_val parameters,
                  wabi_val body)
{
  wabi_combiner_derived res = (wabi_combiner_derived) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
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
wabi_combiner_wrap(wabi_vm vm, wabi_combiner combiner)
{
  wabi_combiner res;
  switch(WABI_TAG(combiner)) {
  case wabi_tag_oper:
    res = (wabi_combiner) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    memcpy(res, combiner, sizeof(wabi_combiner_derived_t));

    /* shortcut  */
    /* *((wabi_word *) res) ^= 0x3000000000000000; */

    *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
    WABI_SET_TAG(res, wabi_tag_app);


    return res;
  case wabi_tag_bt_oper:
    res = (wabi_combiner) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    memcpy(res, combiner, sizeof(wabi_combiner_builtin_t));

    /* shortcut  */
    /* *((wabi_word *) res) ^= 0x9800000000000000; */

    *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
    WABI_SET_TAG(res, wabi_tag_bt_app);

    return res;
  default:
    return (wabi_combiner) combiner;
  }
  return NULL;
}


wabi_combiner
wabi_combiner_unwrap(wabi_vm vm, wabi_combiner combiner)
{
  wabi_combiner res;
  switch(WABI_TAG(combiner)) {
  case wabi_tag_app:
    res = (wabi_combiner) wabi_vm_alloc(vm, WABI_COMBINER_DERIVED_SIZE);
    memcpy(res, combiner, sizeof(wabi_combiner_derived_t));
    *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
    WABI_SET_TAG(res, wabi_tag_oper);
    return res;
  case wabi_tag_bt_app:
    res = (wabi_combiner) wabi_vm_alloc(vm, WABI_COMBINER_BUILTIN_SIZE);
    memcpy(res, combiner, sizeof(wabi_combiner_builtin_t));
    *((wabi_word *) res) = WABI_WORD_VAL(*((wabi_word *) res));
    WABI_SET_TAG(res, wabi_tag_bt_oper);
    return res;
  default:
    return combiner;
  }
  return NULL;
}
