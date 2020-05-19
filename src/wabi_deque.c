#define wabi_deque_c

#include "wabi_deque.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"


/* static inline wabi_deque_digit2 */
/* wabi_deque_node2_to_digit2(wabi_vm vm, wabi_deque_node2 n) */
/* { */
/*   wabi_val a, b; */
/*   a = wabi_deque_node2_l(n); */
/*   b = wabi_deque_node2_r(n); */
/*   return wabi_deque_digit2_new(vm, a, b); */
/* } */


/* static inline wabi_deque_digit3 */
/* wabi_deque_node3_to_digit3(wabi_vm vm, wabi_deque_node3 n) */
/* { */
/*   wabi_val a, b, c; */
/*   a = wabi_deque_node3_l(n); */
/*   b = wabi_deque_node3_m(n); */
/*   c = wabi_deque_node3_r(n); */
/*   return wabi_deque_digit3_new(vm, a, b, c); */
/* } */

static inline wabi_deque_digit
wabi_deque_node_to_digit(wabi_vm vm, wabi_deque_node n)
{
  wabi_val a, b, c;

  switch(WABI_TAG(n)) {
  case wabi_tag_deque_node3:
    a = wabi_deque_node3_l((wabi_deque_node3) n);
    b = wabi_deque_node3_m((wabi_deque_node3) n);
    c = wabi_deque_node3_r((wabi_deque_node3) n);
    return (wabi_deque_digit) wabi_deque_digit3_new(vm, a, b, c);

  case wabi_tag_deque_node2:
    a = wabi_deque_node2_l((wabi_deque_node2) n);
    b = wabi_deque_node2_r((wabi_deque_node2) n);
    return (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
  }
}

static inline wabi_val
wabi_deque_digit_left(wabi_deque_digit d)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_digit1:
    return wabi_deque_digit1_a((wabi_deque_digit1) d);
  case wabi_tag_deque_digit2:
    return wabi_deque_digit2_a((wabi_deque_digit2) d);
  case wabi_tag_deque_digit3:
    return wabi_deque_digit3_a((wabi_deque_digit3) d);
  case wabi_tag_deque_digit4:
    return wabi_deque_digit4_a((wabi_deque_digit4) d);
  }
}


static inline wabi_val
wabi_deque_digit_right(wabi_deque_digit d)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_digit1:
    return wabi_deque_digit1_a((wabi_deque_digit1) d);
  case wabi_tag_deque_digit2:
    return wabi_deque_digit2_b((wabi_deque_digit2) d);
  case wabi_tag_deque_digit3:
    return wabi_deque_digit3_c((wabi_deque_digit3) d);
  case wabi_tag_deque_digit4:
    return wabi_deque_digit4_d((wabi_deque_digit4) d);
  }
}


static inline wabi_deque_digit
wabi_deque_digit_push_left(wabi_vm vm, wabi_deque_digit d, wabi_val x)
{
  wabi_val a, b, c;
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_digit1:
    a = wabi_deque_digit1_a((wabi_deque_digit1) d);
    return (wabi_deque_digit) wabi_deque_digit2_new(vm, x, a);
  case wabi_tag_deque_digit2:
    a = wabi_deque_digit2_a((wabi_deque_digit2) d);
    b = wabi_deque_digit2_b((wabi_deque_digit2) d);
    return (wabi_deque_digit) wabi_deque_digit3_new(vm, x, a, b);
  case wabi_tag_deque_digit3:
    a = wabi_deque_digit3_a((wabi_deque_digit3) d);
    b = wabi_deque_digit3_b((wabi_deque_digit3) d);
    c = wabi_deque_digit3_c((wabi_deque_digit3) d);
    return (wabi_deque_digit) wabi_deque_digit4_new(vm, x, a, b, c);
  }
}


static inline wabi_deque_digit
wabi_deque_digit_push_right(wabi_vm vm, wabi_deque_digit dd, wabi_val x)
{
  wabi_val a, b, c;
  switch(WABI_TAG(dd)) {
  case wabi_tag_deque_digit1:
    a = wabi_deque_digit1_a((wabi_deque_digit1) dd);
    return (wabi_deque_digit) wabi_deque_digit2_new(vm, a, x);
  case wabi_tag_deque_digit2:
    a = wabi_deque_digit2_a((wabi_deque_digit2) dd);
    b = wabi_deque_digit2_b((wabi_deque_digit2) dd);
    return (wabi_deque_digit) wabi_deque_digit3_new(vm, a, b, x);
  case wabi_tag_deque_digit3:
    a = wabi_deque_digit3_a((wabi_deque_digit3) dd);
    b = wabi_deque_digit3_b((wabi_deque_digit3) dd);
    c = wabi_deque_digit3_c((wabi_deque_digit3) dd);
    return (wabi_deque_digit) wabi_deque_digit4_new(vm, a, b, c, x);
  }
}


static inline wabi_deque_digit
wabi_deque_digit_pop_left(wabi_vm vm, wabi_deque_digit dd)
{
  wabi_val b, c, d;
  switch(WABI_TAG(dd)) {
  case wabi_tag_deque_digit2:
    b = wabi_deque_digit2_b((wabi_deque_digit2) dd);
    return (wabi_deque_digit) wabi_deque_digit1_new(vm, b);
  case wabi_tag_deque_digit3:
    b = wabi_deque_digit3_b((wabi_deque_digit3) dd);
    c = wabi_deque_digit3_c((wabi_deque_digit3) dd);
    return (wabi_deque_digit) wabi_deque_digit2_new(vm, b, c);
  case wabi_tag_deque_digit4:
    b = wabi_deque_digit4_b((wabi_deque_digit4) dd);
    c = wabi_deque_digit4_c((wabi_deque_digit4) dd);
    d = wabi_deque_digit4_d((wabi_deque_digit4) dd);
    return (wabi_deque_digit) wabi_deque_digit3_new(vm, b, c, d);
  }
}


static inline wabi_deque_digit
wabi_deque_digit_pop_right(wabi_vm vm, wabi_deque_digit d)
{
  wabi_val a, b, c;
  switch(WABI_TAG(d)) {
  case wabi_tag_deque_digit2:
    a = wabi_deque_digit2_a((wabi_deque_digit2) d);
    return (wabi_deque_digit) wabi_deque_digit1_new(vm, a);
  case wabi_tag_deque_digit3:
    a = wabi_deque_digit3_a((wabi_deque_digit3) d);
    b = wabi_deque_digit3_b((wabi_deque_digit3) d);
    return (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
  case wabi_tag_deque_digit4:
    a = wabi_deque_digit4_a((wabi_deque_digit4) d);
    b = wabi_deque_digit4_b((wabi_deque_digit4) d);
    c = wabi_deque_digit4_c((wabi_deque_digit4) d);
    return (wabi_deque_digit) wabi_deque_digit3_new(vm, a, b, c);
  }
}

// TODO: specialize for left and right pops
static inline wabi_deque
wabi_deque_digit_to_deque(wabi_vm vm, wabi_deque_digit dd)
{
  wabi_val a, b, c, d;
  wabi_deque_digit l, r;
  wabi_deque m;

  switch(WABI_TAG(dd)) {
  case wabi_tag_deque_digit1:
    a = wabi_deque_digit1_a((wabi_deque_digit1) dd);
    return (wabi_deque) wabi_deque_single_new(vm, a);
  case wabi_tag_deque_digit2:
    a = wabi_deque_digit2_a((wabi_deque_digit2) dd);
    b = wabi_deque_digit2_b((wabi_deque_digit2) dd);
    l = (wabi_deque_digit) wabi_deque_digit1_new(vm, a);
    m = (wabi_deque) wabi_deque_empty_new(vm);
    r = (wabi_deque_digit) wabi_deque_digit1_new(vm, b);
    return (wabi_deque) wabi_deque_deep_new(vm, l, m, r);
  case wabi_tag_deque_digit3:
    a = wabi_deque_digit3_a((wabi_deque_digit3) dd);
    b = wabi_deque_digit3_b((wabi_deque_digit3) dd);
    c = wabi_deque_digit3_c((wabi_deque_digit3) dd);
    l = (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
    m = (wabi_deque) wabi_deque_empty_new(vm);
    r = (wabi_deque_digit) wabi_deque_digit1_new(vm, c);
    return (wabi_deque) wabi_deque_deep_new(vm, l, m, r);
  case wabi_tag_deque_digit4:
    a = wabi_deque_digit4_a((wabi_deque_digit4) dd);
    b = wabi_deque_digit4_b((wabi_deque_digit4) dd);
    c = wabi_deque_digit4_c((wabi_deque_digit4) dd);
    d = wabi_deque_digit4_d((wabi_deque_digit4) dd);
    l = (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
    m = (wabi_deque) wabi_deque_empty_new(vm);
    r = (wabi_deque_digit) wabi_deque_digit2_new(vm, c, d);
    return (wabi_deque) wabi_deque_deep_new(vm, l, m, r);
  }
}

// Stuff
wabi_val
wabi_deque_left(wabi_vm vm, wabi_deque dd)
{
  wabi_deque_digit l;

  switch (WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return vm->nil;
  case wabi_tag_deque_single:
    return (wabi_val) wabi_deque_single_val((wabi_deque_single) dd);
  case wabi_tag_deque_deep:
    l = wabi_deque_deep_left((wabi_deque_deep) dd);
    return wabi_deque_digit_left(l);
  }
}


wabi_val
wabi_deque_right(wabi_vm vm, wabi_deque dd)
{
  wabi_deque_digit r;

  switch (WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return vm->nil;
  case wabi_tag_deque_single:
    return (wabi_val) wabi_deque_single_val((wabi_deque_single) dd);
  case wabi_tag_deque_deep:
    r = wabi_deque_deep_right((wabi_deque_deep) dd);
    return wabi_deque_digit_right(r);
  }
}


wabi_deque
wabi_deque_push_left(wabi_vm vm, wabi_val a, wabi_deque dd)
{
  wabi_deque_digit l, r, l1;
  wabi_deque m, de, m1;
  wabi_val b,c,d,e, n1;

  switch(WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return (wabi_deque) wabi_deque_single_new(vm, a);
  case wabi_tag_deque_single:
    b = wabi_deque_single_val((wabi_deque_single) dd);
    l = (wabi_deque_digit) wabi_deque_digit1_new(vm, a);
    if(! l) return NULL;
    r = (wabi_deque_digit) wabi_deque_digit1_new(vm, b);
    if(! r) return NULL;
    de = (wabi_deque) wabi_deque_empty_new(vm);
    return (wabi_deque) wabi_deque_deep_new(vm, l, de, r);
  case wabi_tag_deque_deep:
    l = (wabi_deque_digit) wabi_deque_deep_left((wabi_deque_deep) dd);
    m = (wabi_deque) wabi_deque_deep_middle((wabi_deque_deep) dd);
    r = (wabi_deque_digit) wabi_deque_deep_right((wabi_deque_deep) dd);
    if(WABI_IS(wabi_tag_deque_digit4, l)) {
      b = wabi_deque_digit4_a((wabi_deque_digit4) l);
      c = wabi_deque_digit4_b((wabi_deque_digit4) l);
      d = wabi_deque_digit4_c((wabi_deque_digit4) l);
      e = wabi_deque_digit4_d((wabi_deque_digit4) l);
      l1 = (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
      if(! l1) return NULL;
      n1 = (wabi_val) wabi_deque_node3_new(vm, c, d, e);
      if(! n1) return NULL;
      m1 = wabi_deque_push_left(vm, n1, m);
      if(! m1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l1, m1, r);
    }
    l1 = wabi_deque_digit_push_left(vm, l, a);
    return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
  }
}

wabi_deque
wabi_deque_push_right(wabi_vm vm, wabi_deque dd, wabi_val a)
{
  wabi_deque_digit l, r, r1;
  wabi_deque m, de, m1;
  wabi_val b,c,d,e, n1;

  switch(WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return (wabi_deque) wabi_deque_single_new(vm, a);
  case wabi_tag_deque_single:
    b = wabi_deque_single_val((wabi_deque_single) dd);
    l = (wabi_deque_digit) wabi_deque_digit1_new(vm, b);
    if(! l) return NULL;
    r = (wabi_deque_digit) wabi_deque_digit1_new(vm, a);
    if(! r) return NULL;
    de = (wabi_deque) wabi_deque_empty_new(vm);
    return (wabi_deque) wabi_deque_deep_new(vm, l, de, r);
  case wabi_tag_deque_deep:
    l = (wabi_deque_digit) wabi_deque_deep_left((wabi_deque_deep) dd);
    m = (wabi_deque) wabi_deque_deep_middle((wabi_deque_deep) dd);
    r = (wabi_deque_digit) wabi_deque_deep_right((wabi_deque_deep) dd);
    if(WABI_IS(wabi_tag_deque_digit4, r)) {
      b = wabi_deque_digit4_a((wabi_deque_digit4) r);
      c = wabi_deque_digit4_b((wabi_deque_digit4) r);
      d = wabi_deque_digit4_c((wabi_deque_digit4) r);
      e = wabi_deque_digit4_d((wabi_deque_digit4) r);
      r1 = (wabi_deque_digit) wabi_deque_digit2_new(vm, e, a);
      if(! r1) return NULL;
      n1 = (wabi_val) wabi_deque_node3_new(vm, b, c, d);
      if(! n1) return NULL;
      m1 = wabi_deque_push_right(vm, m, n1);
      if(! m1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l, m1, r1);
    }
    r1 = wabi_deque_digit_push_right(vm, r, a);
    return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1);
  }
}

wabi_deque
wabi_deque_pop_left(wabi_vm vm, wabi_deque dd)
{
  wabi_deque_digit l, r, l1, r1;
  wabi_deque m, m1;
  wabi_val a, b, c, d;
  wabi_deque_node n;

  switch (WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return (wabi_deque) dd;
  case wabi_tag_deque_single:
    return (wabi_deque) wabi_deque_empty_new(vm);
  case wabi_tag_deque_deep:
    l = wabi_deque_deep_left((wabi_deque_deep) dd);
    m = wabi_deque_deep_middle((wabi_deque_deep) dd);
    r = wabi_deque_deep_right((wabi_deque_deep) dd);
    if(WABI_IS(wabi_tag_deque_digit1, l)) {
      n = (wabi_deque_node) wabi_deque_left(vm, m);
      if(wabi_is_nil((wabi_val) n)) return wabi_deque_digit_to_deque(vm, r);
      m1 = wabi_deque_pop_left(vm, m);
      l1 = wabi_deque_node_to_digit(vm, n);
      if(! l1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l1, m1, r);
    }
    l1 = (wabi_deque_digit) wabi_deque_digit_pop_left(vm, l);
    return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
  }
}


wabi_deque
wabi_deque_pop_right(wabi_vm vm, wabi_deque dd)
{
  wabi_deque_digit l, r, l1, r1;
  wabi_deque m, m1;
  wabi_val a, b, c, d;
  wabi_deque_node n;
  switch (WABI_TAG(dd)) {
  case wabi_tag_deque_empty:
    return (wabi_deque) dd;
  case wabi_tag_deque_single:
    return (wabi_deque) wabi_deque_empty_new(vm);
  case wabi_tag_deque_deep:
    l = wabi_deque_deep_left((wabi_deque_deep) dd);
    m = wabi_deque_deep_middle((wabi_deque_deep) dd);
    r = wabi_deque_deep_right((wabi_deque_deep) dd);
    if(WABI_IS(wabi_tag_deque_digit1, r)) {
      n = (wabi_deque_node) wabi_deque_right(vm, m);
      if(wabi_is_nil((wabi_val) n)) return wabi_deque_digit_to_deque(vm, l);
      m1 = wabi_deque_pop_right(vm, m);
      r1 = wabi_deque_node_to_digit(vm, n);
      if(! r1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l, m1, r1);
    }
    r1 = (wabi_deque_digit) wabi_deque_digit_pop_right(vm, r);
    return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1);
  }
}



// builtins
static wabi_error_type
wabi_deque_deq(wabi_vm vm)
{
  wabi_val ctrl, val;
  wabi_deque deq;
  ctrl = vm->ctrl;
  deq = (wabi_deque) wabi_deque_empty_new(vm);
  if(! deq) return wabi_error_nomem;

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    val = wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);
    deq = wabi_deque_push_right(vm, deq, val);
  }
  if(!wabi_is_nil(ctrl))
    return wabi_error_bindings;

  vm->ctrl = (wabi_val) deq;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static inline int
wabi_deque_is_deque(wabi_val val)
{
  switch(WABI_TAG(val)) {
  case wabi_tag_deque_deep:
  case wabi_tag_deque_single:
  case wabi_tag_deque_empty:
    return 1;
  default:
    return 0;
  }
}

static wabi_error_type
wabi_deque_deq_push_right(wabi_vm vm)
{
  wabi_deque dd;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    dd = wabi_deque_push_right(vm, dd, a);
    if(! dd)
      return wabi_error_nomem;
  }
  if(!wabi_is_nil(ctrl)) return wabi_error_type_mismatch;

  vm->ctrl = (wabi_val) dd;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_push_left(wabi_vm vm)
{
  wabi_deque dd;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  a = (wabi_val) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  dd = wabi_deque_push_left(vm, a, dd);
  if(! dd) return wabi_error_nomem;

  vm->ctrl = (wabi_val) dd;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}

static wabi_error_type
wabi_deque_deq_left(wabi_vm vm)
{
  wabi_deque dd;
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  res = wabi_deque_left(vm, dd);
  if(! res) return wabi_error_nomem;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}

static wabi_error_type
wabi_deque_deq_right(wabi_vm vm)
{
  wabi_deque dd;
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  res = wabi_deque_right(vm, dd);
  if(! res) return wabi_error_nomem;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_pop_left(wabi_vm vm)
{

  wabi_deque dd;
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  res = wabi_deque_pop_left(vm, dd);
  if(! res)
    return wabi_error_nomem;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_deque_deq_pop_right(wabi_vm vm)
{
  wabi_deque dd;
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl))
    return wabi_error_bindings;

  dd = (wabi_deque) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_deque_is_deque((wabi_val) dd))
    return wabi_error_type_mismatch;

  res = wabi_deque_pop_right(vm, dd);
  if(! res)
    return wabi_error_nomem;
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}

wabi_error_type
wabi_deque_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "deq", "deq", wabi_deque_deq);
  if(res) return res;
  res = WABI_DEFN(vm, env, "push-left", "push-right", wabi_deque_deq_push_left);
  if(res) return res;
  res = WABI_DEFN(vm, env, "push-right", "push-right", wabi_deque_deq_push_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pop-left", "pop-right", wabi_deque_deq_pop_left);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pop-right", "pop-right", wabi_deque_deq_pop_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "right", "right", wabi_deque_deq_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "left", "left", wabi_deque_deq_left);
  if(res) return res;
}
