/**
 * A combiner is either an applicative or an operative,
 * the lowest bit of the tag is the flag that identifies applicatives.
 *
 * The next bit, i.e. the second is a flag for the builtin/derived functions
 *
 * Although theretically the concept of applicative is derived, it is
 * most effective handling it natively.
 */
#ifndef wabi_combiner_h

#define wabi_combiner_h

#include "wabi_env.h"
#include "wabi_value.h"

typedef struct wabi_combiner_derived_struct {
  wabi_word_t static_env;
  wabi_word_t caller_env_name;
  // TODO: rename to parameters
  wabi_word_t arguments;
  wabi_word_t body;
} wabi_combiner_derived_t;


typedef wabi_combiner_derived_t* wabi_combiner_derived;

typedef wabi_word_t wabi_combiner_builtin_t;

typedef wabi_combiner_builtin_t* wabi_combiner_builtin;


typedef union wabi_combiner_union {
  wabi_combiner_builtin_t builtin;
  wabi_combiner_derived_t derived;
} wabi_combiner_t;

typedef wabi_combiner_t* wabi_combiner;

typedef wabi_val wabi_builtin_fun_type(wabi_vm, wabi_val);

#define WABI_COMBINER_BUILTIN_SIZE 1
#define WABI_COMBINER_DERIVED_SIZE 4

#define WABI_COMBINER_BUILTIN_MASK     0x0200000000000000
#define WABI_COMBINER_APPLICATIVE_MASK 0x0100000000000000
#define WABI_COMBINER_WRAP_MASK        0x0100000000000000
#define WABI_COMBINER_UNWRAP_MASK      0xFEFFFFFFFFFFFFFF

#define WABI_COMBINER_IS_BUILTIN(combiner) (*((wabi_word_t*) (combiner)) & WABI_COMBINER_BUILTIN_MASK)
// #define WABI_COMBINER_IS_APPLICATIVE(combiner) (*((wabi_word_t*) (combiner)) & WABI_COMBINER_APPLICATIVE_MASK)
#define WABI_COMBINER_IS_OPERATIVE(combiner) (!WABI_COMBINER_IS_APPLICATIVE(combiner))

#define WABI_COMBINER_IS_APPLICATIVE(combiner) (*((wabi_val) combiner) & WABI_COMBINER_APPLICATIVE_MASK)

wabi_combiner
wabi_combiner_unwrap(wabi_vm vm, wabi_val combiner);


wabi_combiner
wabi_combiner_wrap(wabi_vm vm, wabi_val combiner);


wabi_combiner
wabi_combiner_builtin_new(wabi_vm vm, wabi_builtin_fun_type fun);


wabi_combiner
wabi_combiner_new(wabi_vm vm,
                  wabi_env static_env,
                  wabi_symbol caller_env_name,
                  wabi_val arguments,
                  wabi_val body);


wabi_val
wabi_combiner_is_operative(wabi_vm vm, wabi_val combiner);


wabi_val
wabi_combiner_is_applicative(wabi_vm vm, wabi_val combiner);

#endif
