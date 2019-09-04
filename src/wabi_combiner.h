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

typedef struct wabi_combiner_derived_struct {
  wabi_word static_env;
  wabi_word caller_env_name;
  // TODO: rename to parameters
  wabi_word arguments;
  wabi_word body;
} wabi_combiner_derived_t;


typedef wabi_combiner_derived_t* wabi_combiner_derived;

typedef wabi_word wabi_combiner_builtin_t;

typedef wabi_combiner_builtin_t* wabi_combiner_builtin;


typedef union wabi_combiner_union {
  wabi_combiner_builtin_t builtin;
  wabi_combiner_derived_t derived;
} wabi_combiner_t;

typedef wabi_combiner_t* wabi_combiner;

#define WABI_COMBINER_BUILTIN_SIZE 1
#define WABI_COMBINER_DERIVED_SIZE 4

wabi_combiner
wabi_combiner_builtin_new(wabi_store store, void* cfun);

wabi_combiner
wabi_combiner_new(wabi_store store,
                  wabi_env static_env,
                  wabi_val caller_env_name,
                  wabi_val arguments,
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
