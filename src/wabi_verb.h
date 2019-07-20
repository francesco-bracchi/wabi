#ifndef wabi_verb_h

#define wabi_verb_h

#include "wabi_env.h"
#include "wabi_value.h"

typedef struct wabi_verb_derived_struct {
  wabi_word_t static_env;
  wabi_word_t dynamic_env_name;
  wabi_word_t arguments;
  wabi_word_t body;
} wabi_verb_derived_t;


typedef wabi_verb_derived_t* wabi_verb_derived;


typedef wabi_word_t wabi_verb_builtin_t;

typedef wabi_verb_builtin_t* wabi_verb_builtin;


typedef union wabi_verb_union {
  wabi_verb_builtin_t builtin;
  wabi_verb_derived_t derived;
} wabi_verb_t;

typedef wabi_verb_t* wabi_verb;

#define WABI_VERB_BUILTIN_SIZE 1
#define WABI_VERB_DERIVED_SIZE 4

#define WABI_VERB_BUILTIN_MASK     0x0200000000000000
#define WABI_VERB_WRAP_MASK        0x0100000000000000
#define WABI_VERB_UNWRAP_MASK      0xFEFFFFFFFFFFFFFF

#define WABI_VERB_IS_BUILTIN(verb) (WABI_VERB_BUILTIN_MASK & ((wabi_word_t) verb))

wabi_verb
wabi_verb_unwrap(wabi_vm vm, wabi_verb verb);


wabi_verb
wabi_verb_wrap(wabi_vm vm, wabi_verb verb);

wabi_verb
wabi_verb_builtin_new(wabi_vm vm, void* fun);

wabi_verb
wabi_verb_new(wabi_vm vm,
              wabi_env static_env,
              wabi_symbol dynamic_env_name,
              wabi_val arguments,
              wabi_val body);

#endif
