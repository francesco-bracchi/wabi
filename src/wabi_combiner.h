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
#include "wabi_pair.h"
#include "wabi_cont.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_error.h"

typedef wabi_error_type (*wabi_builtin_fun)(wabi_vm);

typedef struct wabi_combiner_derived_struct {
  wabi_word static_env;
  wabi_word caller_env_name;
  wabi_word parameters;
  wabi_word body;
  wabi_word compiled_body;
} wabi_combiner_derived_t;


typedef wabi_combiner_derived_t* wabi_combiner_derived;

typedef struct wabi_combiner_builtin_struct {
  wabi_word c_ptr;
  wabi_word c_name;
  wabi_word c_xtra;
} wabi_combiner_builtin_t;

typedef wabi_combiner_builtin_t* wabi_combiner_builtin;

typedef struct wabi_combiner_continuation_struct {
  wabi_word tag;
  wabi_word cont;
  wabi_word prompt;
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
wabi_operator_builtin_new(wabi_vm vm, wabi_binary cname, wabi_builtin_fun cfun);

wabi_combiner
wabi_application_builtin_new(wabi_vm vm, wabi_binary cname, wabi_builtin_fun cfun);

wabi_error_type
wabi_combiner_builtins(wabi_vm vm, wabi_env env);

inline static int
wabi_combiner_is_operative(wabi_val combiner) {
  return WABI_IS(wabi_tag_oper, combiner)
    || WABI_IS(wabi_tag_bt_oper, combiner)
    || WABI_IS(wabi_tag_ct_oper, combiner);
}

static inline int
wabi_combiner_is_applicative(wabi_val combiner) {
  return WABI_IS(wabi_tag_app, combiner)
    || WABI_IS(wabi_tag_bt_app, combiner)
    || WABI_IS(wabi_tag_ct_app, combiner);
}

static inline int
wabi_combiner_is_builtin(wabi_val combiner)
{
  return WABI_IS(wabi_tag_bt_app, combiner) || WABI_IS(wabi_tag_bt_oper, combiner);
}

static inline int
wabi_combiner_is_continuation(wabi_val combiner)
{
  return WABI_IS(wabi_tag_ct_app, combiner) || WABI_IS(wabi_tag_ct_oper, combiner);
}

static inline int
wabi_combiner_is_derived(wabi_val combiner)
{
  return WABI_IS(wabi_tag_app, combiner) || WABI_IS(wabi_tag_oper, combiner);
}

static inline wabi_combiner
wabi_combiner_continuation_new(wabi_vm vm, wabi_val tag, wabi_cont cont, wabi_pair prompt)
{
  wabi_combiner_continuation res = (wabi_combiner_continuation) wabi_vm_alloc(vm, WABI_COMBINER_CONTINUATION_SIZE);
  if(res) {
    res->tag = (wabi_word) tag;
    res->cont = (wabi_word) cont;
    res->prompt = (wabi_word) prompt;
    WABI_SET_TAG(res, wabi_tag_ct_app);
  }
  return (wabi_combiner) res;
}


static inline wabi_val
wabi_combiner_continuation_tag(wabi_combiner_continuation cont)
{
  return (wabi_val) WABI_WORD_VAL(cont->tag);
}


static inline wabi_cont
wabi_combiner_continuation_cont(wabi_combiner_continuation cont)
{
  return (wabi_cont) cont->cont;
}


static inline wabi_pair
wabi_combiner_continuation_prompt(wabi_combiner_continuation cont)
{
  return (wabi_pair) cont->prompt;
}


void
wabi_combiner_copy_val(wabi_store store, wabi_combiner c);


void
wabi_combiner_collect_val(wabi_store store, wabi_combiner c);

#endif
