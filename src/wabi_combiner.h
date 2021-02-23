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
#include "wabi_symbol.h"
#include "wabi_list.h"
#include "wabi_cont.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"

typedef void (*wabi_builtin_fun)(wabi_vm);

typedef struct wabi_combiner_derived_struct {
  wabi_word static_env;
  wabi_word caller_env_name;
  wabi_word parameters;
  wabi_word body;
  wabi_word compiled_body;
} wabi_combiner_derived_t;


typedef wabi_combiner_derived_t* wabi_combiner_derived;

typedef struct wabi_combiner_builtin_struct {
  wabi_word c_xtra;
  wabi_word c_name;
  wabi_word c_ptr;
} wabi_combiner_builtin_t;

typedef wabi_combiner_builtin_t* wabi_combiner_builtin;

typedef struct wabi_combiner_continuation_struct {
  wabi_word cont;
} wabi_combiner_continuation_t;



typedef wabi_combiner_continuation_t* wabi_combiner_continuation;

typedef union wabi_combiner_union {
  wabi_combiner_builtin_t builtin;
  wabi_combiner_derived_t derived;
  wabi_combiner_continuation_t continuation;
} wabi_combiner_t;

typedef wabi_combiner_t* wabi_combiner;

#define WABI_COMBINER_DERIVED_SIZE wabi_sizeof(wabi_combiner_derived_t)
#define WABI_COMBINER_BUILTIN_SIZE wabi_sizeof(wabi_combiner_builtin_t)
#define WABI_COMBINER_CONTINUATION_SIZE wabi_sizeof(wabi_combiner_continuation_t)


wabi_combiner
wabi_operator_builtin_new(const wabi_vm vm, const wabi_binary cname, const wabi_builtin_fun cfun);

wabi_combiner
wabi_application_builtin_new(const wabi_vm vm, const wabi_binary cname, const wabi_builtin_fun cfun);

static inline wabi_val
wabi_combiner_derived_static_env(const wabi_combiner_derived c)
{
  return (wabi_val) WABI_WORD_VAL(c->static_env);
}

static inline wabi_val
wabi_combiner_derived_caller_env_name(const wabi_combiner_derived c)
{
  return (wabi_val) c->caller_env_name;
}

static inline wabi_val
wabi_combiner_derived_parameters(const wabi_combiner_derived c)
{
  return (wabi_val) c->parameters;
}

static inline wabi_val
wabi_combiner_derived_body(const wabi_combiner_derived c)
{
  return (wabi_val) c->body;
}

static inline wabi_val
wabi_combiner_derived_compiled_body(const wabi_combiner_derived c)
{
  return (wabi_val) c->compiled_body;
}

static inline void*
wabi_combiner_builtin_cptr(const wabi_combiner_builtin c)
{
  return (void*) c->c_ptr;
}

static inline wabi_binary
wabi_combiner_builtin_cname(const wabi_combiner_builtin c)
{
  return (wabi_binary) c->c_name;
}

static inline void*
wabi_combiner_builtin_xtra(const wabi_combiner_builtin c)
{
  return (void*) WABI_WORD_VAL(c->c_xtra);
}

void
wabi_combiner_builtins(const wabi_vm vm, const wabi_env env);

inline static int
wabi_combiner_is_operative(const wabi_val combiner) {
  return WABI_IS(wabi_tag_oper, combiner)
    || WABI_IS(wabi_tag_bt_oper, combiner)
    || WABI_IS(wabi_tag_ct_oper, combiner);
}

static inline int
wabi_combiner_is_applicative(const wabi_val combiner) {
  return WABI_IS(wabi_tag_app, combiner)
    || WABI_IS(wabi_tag_bt_app, combiner)
    || WABI_IS(wabi_tag_ct_app, combiner);
}

static inline int
wabi_combiner_is_builtin(const wabi_val combiner)
{
  return WABI_IS(wabi_tag_bt_app, combiner) || WABI_IS(wabi_tag_bt_oper, combiner);
}

static inline int
wabi_combiner_is_continuation(const wabi_val combiner)
{
  return WABI_IS(wabi_tag_ct_app, combiner) || WABI_IS(wabi_tag_ct_oper, combiner);
}

static inline int
wabi_combiner_is_derived(const wabi_val combiner)
{
  return WABI_IS(wabi_tag_app, combiner) || WABI_IS(wabi_tag_oper, combiner);
}

wabi_combiner
wabi_combiner_continuation_new(const wabi_vm vm, const wabi_cont cont);

static inline wabi_cont
wabi_combiner_continuation_cont(const wabi_combiner_continuation cont)
{
  return (wabi_cont) WABI_WORD_VAL(cont->cont);
}

wabi_combiner
wabi_combiner_continuation_new(const wabi_vm vm, const wabi_cont cont);

static inline void
wabi_combiner_hash(const wabi_hash_state state, const wabi_combiner c)
{
  switch(WABI_TAG(c)) {
  case wabi_tag_bt_app:
    wabi_hash_step(state, "C", 1);
  case wabi_tag_bt_oper:
    wabi_hash_step(state, "B", 1);
    wabi_hash_val(state, (wabi_val) ((wabi_combiner_builtin) c)->c_name);
    return;
  case wabi_tag_app:
    wabi_hash_step(state, "C", 1);
  case wabi_tag_oper:
    wabi_hash_step(state, "D", 1);
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_static_env((wabi_combiner_derived) c));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_caller_env_name((wabi_combiner_derived) c));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_parameters((wabi_combiner_derived) c));
    wabi_hash_val(state, (wabi_val) wabi_combiner_derived_body((wabi_combiner_derived) c));
    return;
  case wabi_tag_ct_app:
    wabi_hash_step(state, "C", 1);
  case wabi_tag_ct_oper:
    wabi_hash_step(state, "C", 1);
    wabi_hash_val(state, (wabi_val) wabi_combiner_continuation_cont((wabi_combiner_continuation) c));
    return;
  }
}

#endif
