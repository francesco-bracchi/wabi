#ifndef wabi_cont_h

#define wabi_cont_h

#include "wabi_value.h"
#include "wabi_store.h"
#include "wabi_env.h"

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
  wabi_word controls;
} wabi_cont_prog_t;

typedef wabi_cont_prog_t* wabi_cont_prog;

typedef struct wabi_cont_eval_more_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word data;
  wabi_word done;
} wabi_cont_eval_more_t;

typedef wabi_cont_eval_more_t* wabi_cont_eval_more;

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
  wabi_cont_eval_more_t eval_more;
} wabi_cont_t;

typedef wabi_cont_t* wabi_cont;

#define WABI_CONT_EVAL_SIZE 2
#define WABI_CONT_APPLY_SIZE 3
#define WABI_CONT_CALL_SIZE 3
#define WABI_CONT_SEL_SIZE 4
#define WABI_CONT_EVAL_MORE_SIZE 4
#define WABI_CONT_PROG_SIZE 2
#define WABI_CONT_DEF_SIZE 3

wabi_cont
wabi_cont_eval_new(wabi_store store, wabi_env env, wabi_cont prev);

wabi_cont
wabi_cont_apply_new(wabi_store store, wabi_env env, wabi_val args, wabi_cont prev);

wabi_cont
wabi_cont_call_new(wabi_store store, wabi_env env, wabi_val combiner, wabi_cont prev);

wabi_cont
wabi_cont_sel_new(wabi_store store, wabi_env env, wabi_val left, wabi_val right, wabi_cont prev);

wabi_cont
wabi_cont_eval_more_new(wabi_store store, wabi_env env, wabi_val data, wabi_val done, wabi_cont prev);

wabi_cont
wabi_cont_prog_new(wabi_store store, wabi_env env, wabi_val ctrls, wabi_cont prev);

wabi_cont
wabi_cont_def_new(wabi_store store, wabi_env env, wabi_val val, wabi_cont prev);

#endif
