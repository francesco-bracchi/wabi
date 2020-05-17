#define wabi_deque_c

#include "wabi_deque.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"

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
    switch(WABI_TAG(l)) {
    case wabi_tag_deque_digit4:
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
    case wabi_tag_deque_digit3:
      b = wabi_deque_digit3_a((wabi_deque_digit3) l);
      c = wabi_deque_digit3_b((wabi_deque_digit3) l);
      d = wabi_deque_digit3_c((wabi_deque_digit3) l);
      l1 = (wabi_deque_digit) wabi_deque_digit4_new(vm, a, b, c, d);
      if(!l1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
    case wabi_tag_deque_digit2:
      b = wabi_deque_digit2_a((wabi_deque_digit2) l);
      c = wabi_deque_digit2_b((wabi_deque_digit2) l);
      l1 = (wabi_deque_digit) wabi_deque_digit3_new(vm, a, b, c);
      if(!l1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
    case wabi_tag_deque_digit1:
      b = wabi_deque_digit1_a((wabi_deque_digit1) l);
      l1 = (wabi_deque_digit) wabi_deque_digit2_new(vm, a, b);
      if(!l1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l1, m, r);
    }
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
    switch(WABI_TAG(r)) {
    case wabi_tag_deque_digit4:
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
    case wabi_tag_deque_digit3:
      b = wabi_deque_digit3_a((wabi_deque_digit3) r);
      c = wabi_deque_digit3_b((wabi_deque_digit3) r);
      d = wabi_deque_digit3_c((wabi_deque_digit3) r);
      r1 = (wabi_deque_digit) wabi_deque_digit4_new(vm, b, c, d, a);
      if(!r1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1);
    case wabi_tag_deque_digit2:
      b = wabi_deque_digit2_a((wabi_deque_digit2) r);
      c = wabi_deque_digit2_b((wabi_deque_digit2) r);
      r1 = (wabi_deque_digit) wabi_deque_digit3_new(vm, b, c, a);
      if(!r1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1);
    case wabi_tag_deque_digit1:
      b = wabi_deque_digit1_a((wabi_deque_digit1) r);
      r1 = (wabi_deque_digit) wabi_deque_digit2_new(vm, b, a);
      if(!r1) return NULL;
      return (wabi_deque) wabi_deque_deep_new(vm, l, m, r1);
    }
  }
}

wabi_val
wabi_deque_left(wabi_vm vm, wabi_deque d);

wabi_val
wabi_deque_right(wabi_vm vm, wabi_deque d);

wabi_deque
wabi_deque_pop_left(wabi_vm vm, wabi_deque d);

wabi_deque
wabi_deque_pop_right(wabi_vm vm, wabi_deque d);

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

wabi_error_type
wabi_deque_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "deq", "deq", wabi_deque_deq);
  if(res) return res;
}
