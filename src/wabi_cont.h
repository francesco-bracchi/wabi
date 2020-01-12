#ifndef wabi_cont_h

/**
 * TODO: remove prog?
 */

#define wabi_cont_h

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"

typedef struct wabi_cont_eval_struct {
  wabi_word prev;
  wabi_word env;
} wabi_cont_eval_t;

typedef wabi_cont_eval_t* wabi_cont_eval;

typedef struct wabi_cont_prompt_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word tag;
} wabi_cont_prompt_t;

typedef wabi_cont_prompt_t* wabi_cont_prompt;

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

typedef struct wabi_cont_prog_struct {
  wabi_word prev;
  wabi_word env;
  wabi_word expressions;
} wabi_cont_prog_t;

typedef wabi_cont_prog_t* wabi_cont_prog;

typedef union wabi_cont_union {
  wabi_word prev;
  wabi_cont_eval_t eval;
  wabi_cont_apply_t apply;
  wabi_cont_call_t call;
  wabi_cont_sel_t sel;
  wabi_cont_eval_more_t eval_more;
  wabi_cont_def_t def;
  wabi_cont_prog_t prog;
} wabi_cont_t;

typedef wabi_cont_t* wabi_cont;

#define WABI_CONT_EVAL_SIZE 2
#define WABI_CONT_PROMPT_SIZE 3
#define WABI_CONT_APPLY_SIZE 3
#define WABI_CONT_CALL_SIZE 3
#define WABI_CONT_SEL_SIZE 4
#define WABI_CONT_EVAL_MORE_SIZE 4
#define WABI_CONT_DEF_SIZE 3
#define WABI_CONT_PROG_SIZE 3

static inline wabi_cont
wabi_cont_push_eval(wabi_vm vm, wabi_env env, wabi_cont prev)
{
  wabi_cont_eval cont;

  cont = (wabi_cont_eval) wabi_vm_alloc(vm, WABI_CONT_EVAL_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    WABI_SET_TAG(cont, wabi_tag_cont_eval);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_prompt(wabi_vm vm, wabi_env env, wabi_symbol tag, wabi_cont prev)
{
  wabi_cont_prompt cont;

  cont = (wabi_cont_prompt) wabi_vm_alloc(vm, WABI_CONT_PROMPT_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->tag = (wabi_word) tag;
    WABI_SET_TAG(cont, wabi_tag_cont_prompt);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_apply(wabi_vm vm, wabi_env env, wabi_val args, wabi_cont prev)
{
  wabi_cont_apply cont;

  cont = (wabi_cont_apply) wabi_vm_alloc(vm, WABI_CONT_APPLY_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->args = (wabi_word) args;
    WABI_SET_TAG(cont, wabi_tag_cont_apply);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_call(wabi_vm vm, wabi_env env, wabi_val combiner, wabi_cont prev)
{
  wabi_cont_call cont;

  cont = (wabi_cont_call) wabi_vm_alloc(vm, WABI_CONT_CALL_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->combiner = (wabi_word) combiner;
    WABI_SET_TAG(cont, wabi_tag_cont_call);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_sel(wabi_vm vm, wabi_env env, wabi_val left, wabi_val right, wabi_cont prev)
{
  wabi_cont_sel cont;

  cont = (wabi_cont_sel) wabi_vm_alloc(vm, WABI_CONT_SEL_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->left = (wabi_word) left;
    cont->right = (wabi_word) right;
    WABI_SET_TAG(cont, wabi_tag_cont_sel);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_eval_more(wabi_vm vm, wabi_env env, wabi_val data, wabi_val done, wabi_cont prev)
{
  wabi_cont_eval_more cont;

  cont = (wabi_cont_eval_more) wabi_vm_alloc(vm, WABI_CONT_EVAL_MORE_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->data = (wabi_word) data;
    cont->done = (wabi_word) done;
    WABI_SET_TAG(cont, wabi_tag_cont_eval_more);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_def(wabi_vm vm, wabi_env env, wabi_val pattern, wabi_cont prev)
{
  wabi_cont_def cont;

  cont = (wabi_cont_def) wabi_vm_alloc(vm, WABI_CONT_DEF_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->pattern = (wabi_word) pattern;
    WABI_SET_TAG(cont, wabi_tag_cont_def);
    vm->continuation = (wabi_val) cont;
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_push_prog(wabi_vm vm, wabi_env env, wabi_val expressions, wabi_cont prev)
{
  wabi_cont_prog cont;

  cont = (wabi_cont_prog) wabi_vm_alloc(vm, WABI_CONT_PROG_SIZE);
  if(cont) {
    cont->prev = (wabi_word) prev;
    cont->env = (wabi_word) env;
    cont->expressions = (wabi_word) expressions;
    WABI_SET_TAG(cont, wabi_tag_cont_prog);
  }
  return (wabi_cont) cont;
}

static inline wabi_cont
wabi_cont_prev(wabi_cont cont)
{
  return (wabi_cont) WABI_WORD_VAL(cont->prev);
}

#endif
