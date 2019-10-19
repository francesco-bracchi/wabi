/**
 * A combiner is either an applicative or an operative,
 *
 * Although theretically the concept of applicative is derived, it is
 * most effective handling it natively.
 */
#ifndef wabi_combiner_h

#define wabi_combiner_h

#include "wabi_env.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_binary.h"
#include "wabi_cont.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"

#define WABI_COMBINER_DERIVED_SIZE 4
#define WABI_COMBINER_BUILTIN_SIZE 2

typedef wabi_error_type (*wabi_builtin_fun)(wabi_vm);

typedef struct wabi_combiner_derived_struct {
  wabi_word static_env;
  wabi_word caller_env_name;
  wabi_word parameters;
  wabi_word body;
} wabi_combiner_derived_t;


typedef wabi_combiner_derived_t* wabi_combiner_derived;

typedef struct wabi_combiner_builtin_struct {
  wabi_word c_ptr;
  wabi_word c_name;
} wabi_combiner_builtin_t;

typedef wabi_combiner_builtin_t* wabi_combiner_builtin;

typedef union wabi_combiner_union {
  wabi_combiner_builtin_t builtin;
  wabi_combiner_derived_t derived;
} wabi_combiner_t;

typedef wabi_combiner_t* wabi_combiner;


wabi_combiner
wabi_operator_builtin_new(wabi_vm vm, wabi_binary cname, wabi_builtin_fun cfun);


wabi_combiner
wabi_application_builtin_new(wabi_vm vm, wabi_binary cname, wabi_builtin_fun cfun);


wabi_combiner
wabi_combiner_new(wabi_vm vm,
                  wabi_env static_env,
                  wabi_val caller_env_name,
                  wabi_val parameters,
                  wabi_val body);


inline static int
wabi_combiner_is_operative(wabi_val combiner) {
  return WABI_IS(wabi_tag_oper, combiner) || WABI_IS(wabi_tag_bt_oper, combiner);
}

static inline int
wabi_combiner_is_applicative(wabi_val combiner) {
  return WABI_IS(wabi_tag_app, combiner) || WABI_IS(wabi_tag_bt_app, combiner);
}

static inline int
wabi_combiner_is_builtin(wabi_val combiner)
{
  return WABI_IS(wabi_tag_bt_app, combiner) || WABI_IS(wabi_tag_bt_oper, combiner);
}

static inline int
wabi_combiner_is_derived(wabi_val combiner)
{
  return WABI_IS(wabi_tag_app, combiner) || WABI_IS(wabi_tag_oper, combiner);
}

wabi_error_type
wabi_combiner_builtins(wabi_vm vm, wabi_env env);

#endif
