#define wabi_deque_c

#include "wabi_deque.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"

static const wabi_size wabi_deque_single_max_size = 32;
static const wabi_size wabi_deque_single_mid_size = 16;
static const wabi_size wabi_deque_single_mid_size_plus_one = 16;


static inline wabi_deque_single
wabi_deque_push_empty(wabi_vm vm, wabi_val val)
{
  wabi_deque_single d;
  wabi_word* table;

  d = wabi_deque_single_new(vm, 1L);
  if(d) {
    table = wabi_deque_single_table(d);
    *table = (wabi_word) val;
  }
  return d;
}

static inline wabi_deque
wabi_deque_single_push_right(wabi_vm vm,
                             wabi_deque_single d,
                             wabi_val v)
{
  wabi_deque_single d1, r;
  wabi_deque_empty m;
  wabi_size n;
  wabi_word *t;

  n = wabi_deque_single_node_size(d);
  if(n < wabi_deque_single_max_size) {
    d1 = wabi_deque_single_new(vm, n + 1);
    if(d1) {
      t = wabi_deque_single_table(d1);
      wordcopy(t, wabi_deque_single_table(d), n);
      *(t + n) = (wabi_word) v;
    }
    return (wabi_deque) d1;
  }
  m = wabi_deque_empty_new(vm);
  if(! m) return NULL;

  r = wabi_deque_push_empty(vm, v);
  if(! r) return NULL;

  return (wabi_deque) wabi_deque_deep_new(vm, d, (wabi_deque) m, r);
}


static inline wabi_deque_deep
wabi_deque_deep_push_right(wabi_vm vm, wabi_deque_deep d, wabi_val v)
{
  wabi_deque_single l, r, r1;
  wabi_deque m, m1;
  wabi_size n;
  wabi_word* t;

  l = wabi_deque_deep_left(d);
  m = wabi_deque_deep_middle(d);
  r = wabi_deque_deep_right(d);
  n = wabi_deque_single_node_size(r);
  if (n < wabi_deque_single_max_size) {
    r1 = wabi_deque_single_new(vm, n + 1);
    if (!r1)
      return NULL;
    t = wabi_deque_single_table(r1);
    wordcopy(t, wabi_deque_single_table(r), n);
    *(wabi_deque_single_table(r1) + n) = (wabi_word)v;
    return wabi_deque_deep_new(vm, l, m, r1);
  }
  m1 = wabi_deque_push_right(vm, (wabi_deque) m, (wabi_val) r);
  r1 = wabi_deque_push_empty(vm, v);
  return wabi_deque_deep_new(vm, l, m1, r1);
}


wabi_deque
wabi_deque_push_right(wabi_vm vm, wabi_deque d, wabi_val v)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_deep:
    return (wabi_deque) wabi_deque_deep_push_right(vm, (wabi_deque_deep) d, v);
  case wabi_tag_deque_single:
    return (wabi_deque) wabi_deque_single_push_right(vm, (wabi_deque_single) d, v);
  default:
    return (wabi_deque) wabi_deque_push_empty(vm, v);
  }
}


wabi_val
wabi_deque_left(wabi_vm vm, wabi_deque d)
{
  wabi_deque_single l;
  wabi_word* t;

  switch(WABI_TAG(d)) {
  case wabi_tag_deque_deep:
    l = wabi_deque_deep_left((wabi_deque_deep) d);
    t = wabi_deque_single_table(l);
    return (wabi_val) *t;

  case wabi_tag_deque_single:
    t = wabi_deque_single_table((wabi_deque_single) d);
    return (wabi_val) *t;
  default:
    return vm->nil;
  }
}

static inline wabi_deque
wabi_deque_single_pop_left(wabi_vm vm, wabi_deque_single d)
{
  wabi_size n;
  wabi_word* t;
  wabi_deque_single d1;

  n = wabi_deque_single_node_size(d);
  if(n > 1) {
    n--;
    d1 = wabi_deque_single_new(vm, n);
    if(! d1) return NULL;

    t = wabi_deque_single_table(d1);
    wordcopy(t, wabi_deque_single_table(d) + 1, n);
    return (wabi_deque) d1;
  }
  return (wabi_deque) wabi_deque_empty_new(vm);
}


static inline wabi_deque
wabi_deque_deep_pop_left(wabi_vm vm, wabi_deque_deep d)
{
  wabi_deque_single l1, l, r;
  wabi_deque m, m1;
  wabi_size n;

  l = wabi_deque_deep_left(d);
  m = wabi_deque_deep_middle(d);
  r = wabi_deque_deep_right(d);

  n = wabi_deque_single_node_size(l);
  if(n > 1) {
    l1 = (wabi_deque_single) wabi_deque_single_pop_left(vm, l);
    if(! l1) return NULL;

    return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
  }
  if(wabi_deque_is_empty((wabi_val) m)) {
    return (wabi_deque) r;
  }
  l1 = (wabi_deque_single) wabi_deque_left(vm, m);
  if(! l1) return NULL;

  m1 = wabi_deque_pop_left(vm, m);
  if(! m1) return NULL;

  return (wabi_deque) wabi_deque_deep_new(vm, l1, m1, r);
}


wabi_deque
wabi_deque_pop_left(wabi_vm vm, wabi_deque d)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_deep:
    return wabi_deque_deep_pop_left(vm, (wabi_deque_deep) d);

  case wabi_tag_deque_single:
    return wabi_deque_single_pop_left(vm, (wabi_deque_single) d);
  default:
    return NULL;
  }
}


static wabi_error_type
wabi_deque_deq(wabi_vm vm)
{
  wabi_deque res;
  wabi_val ctrl, a;

  res = (wabi_deque) wabi_deque_empty_new(vm);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_deque_push_right(vm, res, a);
    if(! res)
      return wabi_error_nomem;
  }
  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_push_right(wabi_vm vm)
{
  wabi_deque res;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  res = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_deque((wabi_val) res))
    return wabi_error_type_mismatch;

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_deque_push_right(vm, res, a);
    if(! res)
      return wabi_error_nomem;
  }
  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;

}

static wabi_error_type
wabi_deque_deq_p(wabi_vm vm)
{
  wabi_val res, ctrl, deq;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    deq = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_deque(deq)) {
      *res = wabi_val_false;
      vm->ctrl = res;
      vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
      return wabi_error_none;
    }
  }
  if(!wabi_is_nil(ctrl)) return wabi_error_bindings;
  *res = wabi_val_true;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_left(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_deque d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_deque((wabi_val) d))
    return wabi_error_type_mismatch;

  v = wabi_deque_left(vm, d);

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_pop_left(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_deque d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_deque((wabi_val) d))
    return wabi_error_type_mismatch;

  v = (wabi_val) wabi_deque_pop_left(vm, d);
  if(! v)
    v = (wabi_val) vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


wabi_error_type
wabi_deque_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "deq", "deq", wabi_deque_deq);
  if(res) return res;
  res = WABI_DEFN(vm, env, "deq?", "deq?", wabi_deque_deq_p);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, "deq-len", "deq-len", wabi_deque_deq_len); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-emp?", "deq-emp?", wabi_deque_deq_emp_p); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "push-left", "push-right", wabi_deque_deq_push_left); */
  /* if(res) return res; */
  res = WABI_DEFN(vm, env, "push-right", "push-right", wabi_deque_deq_push_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pop-left", "pop-right", wabi_deque_deq_pop_left);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, "pop-right", "pop-right", wabi_deque_deq_pop_right); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "right", "right", wabi_deque_deq_right); */
  /* if(res) return res; */
  res = WABI_DEFN(vm, env, "left", "left", wabi_deque_deq_left);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, "deq-concat", "deq-concat", wabi_deque_deq_concat); */
  /* if(res) return res; */
}
