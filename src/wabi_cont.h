#ifndef wabi_cont_h

#define wabi_cont_h

#include "wabi_value.h"
#include "wabi_store.h"

typedef struct wabi_cont_struct {
  wabi_word prev;
  wabi_word env;
} wabi_cont_t;

typedef wabi_cont_t* wabi_cont;

typedef struct wabi_cont_eval_struct {
  wabi_cont_t cont;
} wabi_cont_eval_t;

typedef wabi_cont_eval_t* wabi_cont_eval;


typedef struct wabi_cont_apply_struct {
  wabi_cont_t cont;
  wabi_word arguments;
} wabi_cont_apply_t;

typedef wabi_cont_apply_t* wabi_cont_apply;

#define WABI_CONT_EVAL_SIZE 2
#define WABI_CONT_APPLY_SIZE (WABI_CONT_EVAL_SIZE + 1)

wabi_cont
wabi_cont_pop(wabi_store store);

wabi_cont
wabi_cont_eval_push(wabi_store store, wabi_val env);

wabi_cont
wabi_cont_apply_push(wabi_store store, wabi_val env, wabi_val arguments);
#endif
