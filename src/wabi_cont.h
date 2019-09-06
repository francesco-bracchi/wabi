#ifndef wabi_cont_h

#define wabi_cont_h

#include "wabi_value.h"
#include "wabi_store.h"

typedef struct wabi_cont_eval_struct {
  wabi_word prev;
  wabi_word env;
} wabi_cont_eval_t;

typedef wabi_cont_eval_t* wabi_cont_eval;

typedef struct wabi_cont_apply_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word args;
} wabi_cont_apply_t;

typedef wabi_cont_apply_t* wabi_cont_apply;

typedef struct wabi_cont_call_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word combiner;
} wabi_cont_call_t;

typedef wabi_cont_call_t* wabi_cont_call;

typedef struct wabi_cont_sel_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word left;
  wabi_word right;
} wabi_cont_sel_t;

typedef wabi_cont_sel_t* wabi_cont_sel;

typedef struct wabi_cont_prog_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word ctrls;
} wabi_cont_prog_t;

typedef wabi_cont_prog_t* wabi_cont_prog;

typedef struct wabi_cont_eval_all_struct {
  wabi_word prev;
  wabi_word env;
} wabi_cont_eval_all_t;

typedef wabi_cont_eval_all_t* wabi_cont_eval_all;

typedef struct wabi_cont_eval_more_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word data;
  wabi_word done;
} wabi_cont_eval_more_t;

typedef wabi_cont_eval_more_t* wabi_cont_eval_more;

typedef struct wabi_cont_eval_rev_struct {
  wabi_word prev;
  wabi_word data;
} wabi_cont_eval_rev_t;

typedef wabi_cont_eval_rev_t* wabi_cont_eval_rev;

typedef struct wabi_cont_def_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word pattern;
} wabi_cont_def_t;

typedef wabi_cont_def_t* wabi_cont_def;

typedef union wabi_cont_union {
  wabi_word prev;
  wabi_cont_eval_t eval;
  wabi_cont_apply_t apply;
  wabi_cont_call_t call;
  wabi_cont_sel_t sel;
  wabi_cont_prog_t prog;
  wabi_cont_eval_all_t eval_all;
  wabi_cont_eval_more_t eval_more;
  wabi_cont_eval_rev_t eval_rev;
} wabi_cont_t;

typedef wabi_cont_t* wabi_cont;

#define WABI_CONT_EVAL_SIZE 2
#define WABI_CONT_APPLY_SIZE 3
#define WABI_CONT_CALL_SIZE 3
#define WABI_CONT_SEL_SIZE 4
#define WABI_CONT_EVAL_ALL_SIZE 2
#define WABI_CONT_EVAL_MORE_SIZE 4
#define WABI_CONT_EVAL_REV_SIZE 2
#define WABI_CONT_PROG_SIZE 2
#define WABI_CONT_DEF_SIZE 3

wabi_cont
wabi_cont_pop(wabi_store store);

void
wabi_cont_eval_push(wabi_store store, wabi_val env);

void
wabi_cont_apply_push(wabi_store store, wabi_val env, wabi_val args);

void
wabi_cont_call_push(wabi_store store, wabi_val env, wabi_val combiner);

void
wabi_cont_sel_push(wabi_store store, wabi_val env, wabi_val left, wabi_val right);

void
wabi_cont_eval_all_push(wabi_store store, wabi_val env);

void
wabi_cont_eval_more_push(wabi_store store, wabi_val env, wabi_val data, wabi_val done);

void
wabi_cont_eval_rev_push(wabi_store store, wabi_val data);

void
wabi_cont_eval_prog_push(wabi_store store, wabi_val env, wabi_val ctrls);

void
wabi_cont_def_push(wabi_store store, wabi_val env, wabi_val val);

#endif
