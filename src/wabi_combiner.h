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
#include "wabi_store.h"
#include "wabi_binary.h"
#include "wabi_cont.h"

typedef void (*wabi_builtin_fun)(wabi_val*, wabi_store*, wabi_env*, wabi_cont*);

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


#define WABI_COMBINER_BUILTIN_SIZE 2
#define WABI_COMBINER_DERIVED_SIZE 4

wabi_combiner
wabi_combiner_builtin_new(wabi_store store, wabi_binary cname, wabi_builtin_fun cfun);

wabi_combiner
wabi_combiner_new(wabi_store store,
                  wabi_env static_env,
                  wabi_val caller_env_name,
                  wabi_val parameters,
                  wabi_val body);

wabi_combiner
wabi_combiner_wrap(wabi_store store, wabi_combiner combiner);

wabi_combiner
wabi_combiner_unwrap(wabi_store store, wabi_combiner combiner);

wabi_val
wabi_combiner_is_operative(wabi_combiner combiner);

wabi_val
wabi_combiner_is_applicative(wabi_combiner combiner);

#endif
