#define wabi_deque_c

#include "wabi_deque.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"

static const wabi_size wabi_deque_digit_max_size = 32;
static const wabi_size wabi_deque_digit_mid_size = 16;
static const wabi_size wabi_deque_digit_mid_size_plus_one = 16;


wabi_deque
wabi_deque_push_right_generic(wabi_vm vm, wabi_deque d, wabi_val v, wabi_size s);


static inline wabi_deque_digit
wabi_deque_empty_new(wabi_vm vm) {
  return wabi_deque_digit_new(vm, 0, 0);
}


static inline wabi_deque_digit
wabi_deque_single_new(wabi_vm vm,
                      wabi_val v,
                      wabi_size s)
{
  wabi_deque_digit d;
  wabi_val t;

  d = wabi_deque_digit_new(vm, 1, s);
  if(! d) return NULL;
  t = wabi_deque_digit_table(d);
  *t = (wabi_word) v;
  return d;
}


static inline wabi_deque
wabi_deque_digit_push_right(wabi_vm vm,
                            wabi_deque_digit d0,
                            wabi_val v,
                            wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t, t0;
  wabi_deque_digit d, r;
  wabi_deque m;
  n0 = wabi_deque_digit_node_size(d0);
  s0 = wabi_deque_size((wabi_deque) d0);
  t0 = wabi_deque_digit_table(d0);
  if (n0 < wabi_deque_digit_max_size) {
    d = wabi_deque_digit_new(vm, n0 + 1L, s0 + s);
    if (!d)
      return NULL;
    t = wabi_deque_digit_table(d);
    wordcopy(t, t0, n0);
    *(t + n0) = (wabi_word)v;
    return (wabi_deque)d;
  }
  m = (wabi_deque)wabi_deque_empty_new(vm);
  r = wabi_deque_single_new(vm, v, s);
  if (!m)
    return NULL;

  return (wabi_deque) wabi_deque_deep_new(vm, d, (wabi_deque) m, r);
}

static inline wabi_deque_deep
wabi_deque_deep_push_right(wabi_vm vm,
                           wabi_deque_deep d0,
                           wabi_val v,
                           wabi_size s)
{

  wabi_deque_digit l0, r0, r;
  wabi_deque m0, m;
  wabi_size s0, n0;
  wabi_val t0, t;

  l0 = wabi_deque_deep_left(d0);
  m0 = wabi_deque_deep_middle(d0);
  r0 = wabi_deque_deep_right(d0);
  n0 = wabi_deque_digit_node_size(r0);
  s0 = wabi_deque_size((wabi_deque) r0);

  if (n0 < wabi_deque_digit_max_size) {
    t0 = wabi_deque_digit_table(r0);
    r = wabi_deque_digit_new(vm, n0 + 1L, s + s0);
    if(! r) return NULL;
    t = wabi_deque_digit_table(r);
    wordcopy(t0, t, n0);
    *(t + n0) = (wabi_word) v;
    return wabi_deque_deep_new(vm, l0, m0, r);
  }
  m = wabi_deque_push_right_generic(vm, m0, v, s0);
  if(! m) return NULL;
  r = wabi_deque_single_new(vm, v, s);
  if(! r) return NULL;
  return wabi_deque_deep_new(vm, l0, m, r);

}

wabi_deque
wabi_deque_push_right_generic(wabi_vm vm, wabi_deque d, wabi_val v, wabi_size s)
{
  if(WABI_IS(wabi_tag_deque_digit, d)) {
    return (wabi_deque) wabi_deque_digit_push_right(vm, (wabi_deque_digit) d, v, s);
  }
  return (wabi_deque) wabi_deque_deep_push_right(vm, (wabi_deque_deep) d, v, s);
}

wabi_deque
wabi_deque_push_right(wabi_vm vm,
                      wabi_deque d,
                      wabi_val v)
{
  return (wabi_deque) wabi_deque_push_right_generic(vm, d, v, 1L);
}

/* // -------------------------------------------------------------------------- */


/* static inline wabi_deque */
/* wabi_deque_digit_push_left(wabi_vm vm, */
/*                             wabi_val v, */
/*                             wabi_deque_digit d) */
/* { */
/*   wabi_deque_digit d1, l; */
/*   wabi_deque_empty m; */
/*   wabi_size n; */
/*   wabi_word *t; */

/*   n = wabi_deque_digit_node_size(d); */
/*   if(n < wabi_deque_digit_max_size) { */
/*     d1 = wabi_deque_digit_new(vm, n + 1); */
/*     if(d1) { */
/*       t = wabi_deque_digit_table(d1); */
/*       *t = (wabi_word) v; */
/*       wordcopy(t + 1, wabi_deque_digit_table(d), n); */
/*     } */
/*     return (wabi_deque) d1; */
/*   } */
/*   m = wabi_deque_empty_new(vm); */
/*   if(! m) return NULL; */

/*   l = wabi_deque_push_empty(vm, v); */
/*   if(! l) return NULL; */

/*   return (wabi_deque) wabi_deque_deep_new(vm, l, (wabi_deque) m, d); */
/* } */


/* static inline wabi_deque_deep */
/* wabi_deque_deep_push_right(wabi_vm vm, wabi_deque_deep d, wabi_val v) */
/* { */
/*   wabi_deque_digit l, r, r1; */
/*   wabi_deque m, m1; */
/*   wabi_size n; */
/*   wabi_word* t; */

/*   l = wabi_deque_deep_left(d); */
/*   m = wabi_deque_deep_middle(d); */
/*   r = wabi_deque_deep_right(d); */
/*   n = wabi_deque_digit_node_size(r); */
/*   if (n < wabi_deque_digit_max_size) { */
/*     r1 = wabi_deque_digit_new(vm, n + 1); */
/*     if (!r1) */
/*       return NULL; */
/*     t = wabi_deque_digit_table(r1); */
/*     wordcopy(t, wabi_deque_digit_table(r), n); */
/*     *(t + n) = (wabi_word)v; */
/*     return wabi_deque_deep_new(vm, l, m, r1); */
/*   } */
/*   m1 = wabi_deque_push_right(vm, (wabi_deque) m, (wabi_val) r); */
/*   if(! m1) return NULL; */
/*   r1 = wabi_deque_push_empty(vm, v); */
/*   if(! r1) return NULL; */
/*   return wabi_deque_deep_new(vm, l, m1, r1); */
/* } */

/* static inline wabi_deque_deep */
/* wabi_deque_deep_push_left(wabi_vm vm, wabi_val v, wabi_deque_deep d) */
/* { */
/*   wabi_deque_digit l, r, l1; */
/*   wabi_deque m, m1; */
/*   wabi_size n; */
/*   wabi_word* t; */

/*   l = wabi_deque_deep_left(d); */
/*   m = wabi_deque_deep_middle(d); */
/*   r = wabi_deque_deep_right(d); */
/*   n = wabi_deque_digit_node_size(r); */
/*   if (n < wabi_deque_digit_max_size) { */
/*     l1 = wabi_deque_digit_new(vm, n + 1); */
/*     if (!l1) return NULL; */
/*     t = wabi_deque_digit_table(l1); */
/*     *t = (wabi_word) v; */
/*     wordcopy(t + 1, wabi_deque_digit_table(l), n); */
/*     return wabi_deque_deep_new(vm, l1, m, r); */
/*   } */
/*   m1 = wabi_deque_push_left(vm, (wabi_val) l, (wabi_deque) m); */
/*   if(! m1) return NULL; */
/*   l1 = wabi_deque_push_empty(vm, v); */
/*   if(! l1) return NULL; */
/*   return wabi_deque_deep_new(vm, l1, m1, r); */
/* } */


/* wabi_deque */
/* wabi_deque_push_right(wabi_vm vm, wabi_deque d, wabi_val v) */
/* { */
/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     return (wabi_deque) wabi_deque_deep_push_right(vm, (wabi_deque_deep) d, v); */
/*   case wabi_tag_deque_digit: */
/*     return (wabi_deque) wabi_deque_digit_push_right(vm, (wabi_deque_digit) d, v); */
/*   default: */
/*     return (wabi_deque) wabi_deque_push_empty(vm, v); */
/*   } */
/* } */


/* wabi_deque */
/* wabi_deque_push_left(wabi_vm vm, wabi_val v, wabi_deque d) */
/* { */
/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     return (wabi_deque) wabi_deque_deep_push_left(vm, v, (wabi_deque_deep) d); */
/*   case wabi_tag_deque_digit: */
/*     return (wabi_deque) wabi_deque_digit_push_left(vm, v, (wabi_deque_digit) d); */
/*   default: */
/*     return (wabi_deque) wabi_deque_push_empty(vm, v); */
/*   } */
/* } */


/* wabi_val */
/* wabi_deque_left(wabi_vm vm, wabi_deque d) */
/* { */
/*   wabi_deque_digit l; */
/*   wabi_word* t; */

/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     l = wabi_deque_deep_left((wabi_deque_deep) d); */
/*     t = wabi_deque_digit_table(l); */
/*     return (wabi_val) *t; */

/*   case wabi_tag_deque_digit: */
/*     t = wabi_deque_digit_table((wabi_deque_digit) d); */
/*     return (wabi_val) *t; */
/*   default: */
/*     return vm->nil; */
/*   } */
/* } */

/* wabi_val */
/* wabi_deque_right(wabi_vm vm, wabi_deque d) */
/* { */
/*   wabi_deque_digit r; */
/*   wabi_size n; */
/*   wabi_word* t; */

/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     r = wabi_deque_deep_right((wabi_deque_deep) d); */
/*     n = wabi_deque_digit_node_size(r); */
/*     t = wabi_deque_digit_table(r); */

/*     return (wabi_val) *(t + n - 1); */

/*   case wabi_tag_deque_digit: */
/*     t = wabi_deque_digit_table((wabi_deque_digit) d); */
/*     n = wabi_deque_digit_node_size((wabi_deque_digit) d); */
/*     return (wabi_val) *(t + n - 1); */
/*   default: */
/*     return vm->nil; */
/*   } */
/* } */

/* static inline wabi_deque */
/* wabi_deque_digit_pop_left(wabi_vm vm, wabi_deque_digit d) */
/* { */
/*   wabi_size n; */
/*   wabi_word* t; */
/*   wabi_deque_digit d1; */

/*   n = wabi_deque_digit_node_size(d); */
/*   if(n > 1) { */
/*     n--; */
/*     d1 = wabi_deque_digit_new(vm, n); */
/*     if(! d1) return NULL; */

/*     t = wabi_deque_digit_table(d1); */
/*     wordcopy(t, wabi_deque_digit_table(d) + 1, n); */
/*     return (wabi_deque) d1; */
/*   } */
/*   return (wabi_deque) wabi_deque_empty_new(vm); */
/* } */


/* static inline wabi_deque */
/* wabi_deque_deep_pop_left(wabi_vm vm, wabi_deque_deep d) */
/* { */
/*   wabi_deque_digit l1, l, r; */
/*   wabi_deque m, m1; */
/*   wabi_size n; */

/*   l = wabi_deque_deep_left(d); */
/*   m = wabi_deque_deep_middle(d); */
/*   r = wabi_deque_deep_right(d); */

/*   n = wabi_deque_digit_node_size(l); */
/*   if(n > 1) { */
/*     l1 = (wabi_deque_digit) wabi_deque_digit_pop_left(vm, l); */
/*     if(! l1) return NULL; */

/*     return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r); */
/*   } */
/*   if(wabi_deque_is_empty((wabi_val) m)) { */
/*     return (wabi_deque) r; */
/*   } */
/*   l1 = (wabi_deque_digit) wabi_deque_left(vm, m); */
/*   if(! l1) return NULL; */

/*   m1 = wabi_deque_pop_left(vm, m); */
/*   if(! m1) return NULL; */

/*   return (wabi_deque) wabi_deque_deep_new(vm, l1, m1, r); */
/* } */


/* wabi_deque */
/* wabi_deque_pop_left(wabi_vm vm, wabi_deque d) */
/* { */
/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     return wabi_deque_deep_pop_left(vm, (wabi_deque_deep) d); */

/*   case wabi_tag_deque_digit: */
/*     return wabi_deque_digit_pop_left(vm, (wabi_deque_digit) d); */
/*   default: */
/*     return NULL; */
/*   } */
/* } */

/* static inline wabi_deque */
/* wabi_deque_digit_pop_right(wabi_vm vm, */
/*                             wabi_deque_digit d) */
/* { */
/*   wabi_size n; */
/*   wabi_word* t; */
/*   wabi_deque_digit d1; */

/*   n = wabi_deque_digit_node_size(d); */
/*   if(n > 1) { */
/*     n--; */
/*     d1 = wabi_deque_digit_new(vm, n); */
/*     if(! d1) return NULL; */

/*     t = wabi_deque_digit_table(d1); */
/*     wordcopy(t, wabi_deque_digit_table(d), n); */
/*     return (wabi_deque) d1; */
/*   } */
/*   return (wabi_deque) wabi_deque_empty_new(vm); */
/* } */


/* static inline wabi_deque */
/* wabi_deque_deep_pop_right(wabi_vm vm, wabi_deque_deep d) */
/* { */
/*   wabi_deque_digit r1, l, r; */
/*   wabi_deque m, m1; */
/*   wabi_size n; */

/*   l = wabi_deque_deep_left(d); */
/*   m = wabi_deque_deep_middle(d); */
/*   r = wabi_deque_deep_right(d); */

/*   n = wabi_deque_digit_node_size(r); */
/*   if(n > 1) { */
/*     r1 = (wabi_deque_digit) wabi_deque_digit_pop_right(vm, r); */
/*     if(! r1) return NULL; */

/*     return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1); */
/*   } */
/*   if(wabi_deque_is_empty((wabi_val) m)) { */
/*     return (wabi_deque) l; */
/*   } */
/*   r1 = (wabi_deque_digit) wabi_deque_right(vm, m); */
/*   if(! r1) return NULL; */

/*   m1 = wabi_deque_pop_right(vm, m); */
/*   if(! m1) return NULL; */

/*   return (wabi_deque) wabi_deque_deep_new(vm, l, m1, r1); */
/* } */


/* wabi_deque */
/* wabi_deque_pop_right(wabi_vm vm, wabi_deque d) */
/* { */
/*   switch(WABI_TAG(d)) { */
/*   case wabi_tag_deque_deep: */
/*     return wabi_deque_deep_pop_right(vm, (wabi_deque_deep) d); */

/*   case wabi_tag_deque_digit: */
/*     return wabi_deque_digit_pop_right(vm, (wabi_deque_digit) d); */
/*   default: */
/*     return NULL; */
/*   } */
/* } */


/* static inline int */
/* wabi_deque_is_single(wabi_deque d) */
/* { */
/*   return WABI_IS(wabi_tag_deque_digit, d); */
/* } */


/* wabi_deque */
/* wabi_deque_concat(wabi_vm vm, wabi_deque l, wabi_deque r) */
/* { */
/*   wabi_deque_digit l1, r1, l2, r2; */
/*   wabi_deque m1, m2, m; */

/*   if(wabi_deque_is_empty((wabi_val) l)) { */
/*     // [] . r */
/*     return r; */
/*   } */
/*   if(wabi_deque_is_empty((wabi_val) r)) { */
/*     // l . [] */
/*     return l; */
/*   } */
/*   if(wabi_deque_is_single(l) && wabi_deque_is_single(r)) { */
/*     // l . r */
/*     m1 = (wabi_deque) wabi_deque_empty_new(vm); */
/*     if(! m1) return NULL; */

/*     return (wabi_deque) wabi_deque_deep_new(vm, (wabi_deque_digit) l, m1, (wabi_deque_digit) r); */
/*   } */
/*   if(wabi_deque_is_single(l)) { */
/*     // l . l2 m2 r2 */
/*     l2 = wabi_deque_deep_left((wabi_deque_deep) r); */
/*     m2 = wabi_deque_deep_middle((wabi_deque_deep) r); */
/*     r2 = wabi_deque_deep_right((wabi_deque_deep) r); */
/*     m2 = (wabi_deque) wabi_deque_push_left(vm, (wabi_val) l2, m2); */
/*     if(! m2) return NULL; */

/*     return (wabi_deque) wabi_deque_deep_new(vm, (wabi_deque_digit) l, m2, r2); */
/*   } */
/*   if(wabi_deque_is_single(r)) { */
/*     // l1 m1 r1 . r */
/*     l1 = wabi_deque_deep_left((wabi_deque_deep) l); */
/*     m1 = wabi_deque_deep_middle((wabi_deque_deep) l); */
/*     r1 = wabi_deque_deep_right((wabi_deque_deep) l); */
/*     m1 = (wabi_deque) wabi_deque_push_right(vm, m1, (wabi_val) r1); */
/*     if(! m1) return NULL; */

/*     return (wabi_deque) wabi_deque_deep_new(vm, l1, m1, (wabi_deque_digit) r); */
/*   } */
/*   // l1 m1 r1 . l2 m2 r2 */

/*   l1 = wabi_deque_deep_left((wabi_deque_deep) l); */
/*   m1 = wabi_deque_deep_middle((wabi_deque_deep) l); */
/*   r1 = wabi_deque_deep_right((wabi_deque_deep) l); */

/*   l2 = wabi_deque_deep_left((wabi_deque_deep) r); */
/*   m2 = wabi_deque_deep_middle((wabi_deque_deep) r); */
/*   r2 = wabi_deque_deep_right((wabi_deque_deep) r); */

/*   m1 = wabi_deque_push_right(vm, m1, (wabi_val) r1); */
/*   if(! m1) return NULL; */

/*   m2 = wabi_deque_push_left(vm, (wabi_val) l2, m2); */
/*   if(! m2) return NULL; */

/*   m = wabi_deque_concat(vm, m1, m2); */
/*   if(! m) return NULL; */

/*   return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r2); */
/* } */


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


/* static wabi_error_type */
/* wabi_deque_deq_push_right(wabi_vm vm) */
/* { */
/*   wabi_deque res; */
/*   wabi_val ctrl, a; */

/*   ctrl = vm->ctrl; */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   res = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_deque((wabi_val) res)) */
/*     return wabi_error_type_mismatch; */

/*   while(wabi_is_pair(ctrl)) { */
/*     a = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     res = wabi_deque_push_right(vm, res, a); */
/*     if(! res) */
/*       return wabi_error_nomem; */
/*   } */
/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   vm->ctrl = (wabi_val) res; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_push_left(wabi_vm vm) */
/* { */
/*   wabi_deque d; */
/*   wabi_val ctrl, v; */

/*   ctrl = vm->ctrl; */
/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   v = wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_deque((wabi_val) d)) */
/*     return wabi_error_type_mismatch; */

/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = wabi_deque_push_left(vm, v, d); */
/*   if(! d) */
/*     return wabi_error_nomem; */

/*   vm->ctrl = (wabi_val) d; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */

/* } */


/* static wabi_error_type */
/* wabi_deque_deq_p(wabi_vm vm) */
/* { */
/*   wabi_val res, ctrl, deq; */
/*   res = (wabi_val) wabi_vm_alloc(vm, 1); */
/*   if(! res) return wabi_error_nomem; */

/*   ctrl = vm->ctrl; */
/*   while(WABI_IS(wabi_tag_pair, ctrl)) { */
/*     deq = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(! wabi_is_deque(deq)) { */
/*       *res = wabi_val_false; */
/*       vm->ctrl = res; */
/*       vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*       return wabi_error_none; */
/*     } */
/*   } */
/*   if(!wabi_is_nil(ctrl)) return wabi_error_bindings; */
/*   *res = wabi_val_true; */
/*   vm->ctrl = res; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_left(wabi_vm vm) */
/* { */
/*   wabi_val v, ctrl; */
/*   wabi_deque d; */
/*   ctrl = vm->ctrl; */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   if(! wabi_is_deque((wabi_val) d)) */
/*     return wabi_error_type_mismatch; */

/*   v = wabi_deque_left(vm, d); */

/*   vm->ctrl = v; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_right(wabi_vm vm) */
/* { */
/*   wabi_val v, ctrl; */
/*   wabi_deque d; */
/*   ctrl = vm->ctrl; */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   if(! wabi_is_deque((wabi_val) d)) */
/*     return wabi_error_type_mismatch; */

/*   v = wabi_deque_right(vm, d); */

/*   vm->ctrl = v; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_pop_left(wabi_vm vm) */
/* { */
/*   wabi_val v, ctrl; */
/*   wabi_deque d; */
/*   ctrl = vm->ctrl; */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   if(! wabi_is_deque((wabi_val) d)) */
/*     return wabi_error_type_mismatch; */

/*   v = (wabi_val) wabi_deque_pop_left(vm, d); */
/*   if(! v) */
/*     v = (wabi_val) vm->nil; */

/*   vm->ctrl = v; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_pop_right(wabi_vm vm) */
/* { */
/*   wabi_val v, ctrl; */
/*   wabi_deque d; */
/*   ctrl = vm->ctrl; */

/*   if(! wabi_is_pair(ctrl)) */
/*     return wabi_error_bindings; */

/*   d = (wabi_deque) wabi_car((wabi_pair) ctrl); */
/*   ctrl = wabi_cdr((wabi_pair) ctrl); */

/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   if(! wabi_is_deque((wabi_val) d)) */
/*     return wabi_error_type_mismatch; */

/*   v = (wabi_val) wabi_deque_pop_right(vm, d); */
/*   if(! v) */
/*     v = (wabi_val) vm->nil; */

/*   vm->ctrl = v; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */


/* static wabi_error_type */
/* wabi_deque_deq_emp_p(wabi_vm vm) */
/* { */
/*   wabi_val res, ctrl, deq; */
/*   res = (wabi_val) wabi_vm_alloc(vm, 1); */
/*   if(! res) return wabi_error_nomem; */

/*   ctrl = vm->ctrl; */
/*   while(WABI_IS(wabi_tag_pair, ctrl)) { */
/*     deq = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(! wabi_deque_is_empty(deq)) { */
/*       *res = wabi_val_false; */
/*       vm->ctrl = res; */
/*       vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*       return wabi_error_none; */
/*     } */
/*   } */
/*   if(!wabi_is_nil(ctrl)) return wabi_error_bindings; */
/*   *res = wabi_val_true; */
/*   vm->ctrl = res; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */

/* static wabi_error_type */
/* wabi_deque_deq_concat(wabi_vm vm) */
/* { */
/*   wabi_val ctrl, d; */
/*   wabi_deque r; */

/*   ctrl = vm->ctrl; */
/*   r = (wabi_deque) wabi_deque_empty_new(vm); */
/*   if(! r) return wabi_error_nomem; */

/*   while(wabi_is_pair(ctrl)) { */
/*     d = wabi_car((wabi_pair) ctrl); */
/*     ctrl = wabi_cdr((wabi_pair) ctrl); */
/*     if(! wabi_is_deque(d)) */
/*       return wabi_error_type_mismatch; */

/*     r = wabi_deque_concat(vm, r, (wabi_deque) d); */
/*     if(! r) */
/*       return wabi_error_nomem; */
/*   } */
/*   if(! wabi_is_nil(ctrl)) */
/*     return wabi_error_bindings; */

/*   vm->ctrl = (wabi_val) r; */
/*   vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont); */
/*   return wabi_error_none; */
/* } */



wabi_error_type
wabi_deque_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "deq", "deq", wabi_deque_deq);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, "deq?", "deq?", wabi_deque_deq_p); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-len", "deq-len", wabi_deque_deq_len); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-emp?", "deq-emp?", wabi_deque_deq_emp_p); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "push-left", "push-right", wabi_deque_deq_push_left); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "push-right", "push-right", wabi_deque_deq_push_right); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "pop-left", "pop-right", wabi_deque_deq_pop_left); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "pop-right", "pop-right", wabi_deque_deq_pop_right); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "right", "right", wabi_deque_deq_right); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "left", "left", wabi_deque_deq_left); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-concat", "deq-concat", wabi_deque_deq_concat); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-ref", "deq-ref", wabi_deque_deq_ref); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-assoc", "deq-assoc", wabi_deque_deq_assoc); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "deq-sub", "deq-sub", wabi_deque_deq_sub); */
  /* if(res) return res; */
}
