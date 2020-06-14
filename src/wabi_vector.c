#define wabi_vector_c

#include "wabi_vector.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_constant.h"
#include "wabi_number.h"


static const wabi_size wabi_vector_digit_max_size = 32;
static const wabi_size wabi_vector_digit_mid_size = 16;
static const wabi_size wabi_vector_digit_mid_size_plus_one = 16;


static inline wabi_vector_digit
wabi_vector_empty_new(wabi_vm vm) {
  return wabi_vector_digit_new(vm, 0, 0);
}


static inline wabi_vector_digit
wabi_vector_single_new(wabi_vm vm,
                      wabi_val v,
                      wabi_size s)
{
  wabi_vector_digit d;
  wabi_val t;

  d = wabi_vector_digit_new(vm, 1L, s);
  if(! d) return NULL;
  t = wabi_vector_digit_table(d);
  *t = (wabi_word) v;
  return d;
}


static wabi_vector
wabi_vector_push_right_generic(wabi_vm vm, wabi_vector d, wabi_val v, wabi_size s);



static inline wabi_vector
wabi_vector_digit_push_right(wabi_vm vm,
                            wabi_vector_digit d0,
                            wabi_val v,
                            wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t, t0;
  wabi_vector_digit d, r;
  wabi_vector m;
  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  t0 = wabi_vector_digit_table(d0);
  if (n0 < wabi_vector_digit_max_size) {
    d = wabi_vector_digit_new(vm, n0 + 1L, s0 + s);
    if (!d)
      return NULL;
    t = wabi_vector_digit_table(d);
    wordcopy(t, t0, n0);
    *(t + n0) = (wabi_word)v;
    return (wabi_vector)d;
  }
  m = (wabi_vector)wabi_vector_empty_new(vm);
  if(! m)
    return NULL;

  r = wabi_vector_single_new(vm, v, s);
  if (!r)
    return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, d0, (wabi_vector) m, r);
}


static inline wabi_vector_deep
wabi_vector_deep_push_right(wabi_vm vm,
                           wabi_vector_deep d0,
                           wabi_val v,
                           wabi_size s)
{

  wabi_vector_digit l0, r0, r;
  wabi_vector m0, m;
  wabi_size sr, nr;
  wabi_val tr, t;

  l0 = wabi_vector_deep_left(d0);
  m0 = wabi_vector_deep_middle(d0);
  r0 = wabi_vector_deep_right(d0);
  nr = wabi_vector_digit_node_size(r0);
  sr = wabi_vector_size((wabi_vector) r0);
  if (nr < wabi_vector_digit_max_size) {
    tr = wabi_vector_digit_table(r0);
    r = wabi_vector_digit_new(vm, nr + 1L, s + sr);
    if(! r) return NULL;
    t = wabi_vector_digit_table(r);
    wordcopy(t, tr, nr);
    *(t + nr) = (wabi_word) v;
    return wabi_vector_deep_new(vm, l0, m0, r);
  }
  m = wabi_vector_push_right_generic(vm, m0, (wabi_val) r0, sr);
  if(! m) return NULL;
  r = wabi_vector_single_new(vm, v, s);
  if(! r) return NULL;
  return wabi_vector_deep_new(vm, l0, m, r);

}


static wabi_vector
wabi_vector_push_right_generic(wabi_vm vm, wabi_vector d, wabi_val v, wabi_size s)
{
  if(WABI_IS(wabi_tag_vector_digit, d)) {
    return (wabi_vector) wabi_vector_digit_push_right(vm, (wabi_vector_digit) d, v, s);
  }
  return (wabi_vector) wabi_vector_deep_push_right(vm, (wabi_vector_deep) d, v, s);
}


wabi_vector
wabi_vector_push_right(wabi_vm vm,
                      wabi_vector d,
                      wabi_val v)
{
  return (wabi_vector) wabi_vector_push_right_generic(vm, d, v, 1L);
}


static wabi_vector
wabi_vector_push_left_generic(wabi_vm vm,
                             wabi_val v,
                             wabi_vector d,
                             wabi_size s);



static inline wabi_vector
wabi_vector_digit_push_left(wabi_vm vm,
                           wabi_val v,
                           wabi_vector_digit d0,
                           wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t, t0;
  wabi_vector_digit d, l;
  wabi_vector m;
  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  t0 = wabi_vector_digit_table(d0);
  if (n0 < wabi_vector_digit_max_size) {
    d = wabi_vector_digit_new(vm, n0 + 1L, s0 + s);
    if (!d)
      return NULL;
    t = wabi_vector_digit_table(d);
    *t = (wabi_word)v;
    wordcopy(t + 1, t0, n0);
    return (wabi_vector)d;
  }
  m = (wabi_vector)wabi_vector_empty_new(vm);
  if (!m)
    return NULL;

  l = wabi_vector_single_new(vm, v, s);
  if(!l)
    return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l, (wabi_vector) m, d0);
}


static inline wabi_vector_deep
wabi_vector_deep_push_left(wabi_vm vm,
                           wabi_val v,
                           wabi_vector_deep d0,
                           wabi_size s)
{

  wabi_vector_digit l0, r0, l;
  wabi_vector m0, m;
  wabi_size sl, nl;
  wabi_val tl, t;

  l0 = wabi_vector_deep_left(d0);
  m0 = wabi_vector_deep_middle(d0);
  r0 = wabi_vector_deep_right(d0);
  nl = wabi_vector_digit_node_size(l0);
  sl = wabi_vector_size((wabi_vector) l0);
  if (nl < wabi_vector_digit_max_size) {
    tl = wabi_vector_digit_table(l0);
    l = wabi_vector_digit_new(vm, nl + 1L, s + sl);
    if(! l) return NULL;
    t = wabi_vector_digit_table(l);
    *t = (wabi_word)v;
    wordcopy(t + 1, tl, nl);
    return wabi_vector_deep_new(vm, l, m0, r0);
  }
  m = wabi_vector_push_left_generic(vm, (wabi_val) l0, m0, sl);
  if(! m) return NULL;
  l = wabi_vector_single_new(vm, v, s);
  if(! l) return NULL;
  return wabi_vector_deep_new(vm, l, m, r0);

}


static wabi_vector
wabi_vector_push_left_generic(wabi_vm vm,
                             wabi_val v,
                             wabi_vector d,
                             wabi_size s)
{
  if(WABI_IS(wabi_tag_vector_digit, d)) {
    return (wabi_vector) wabi_vector_digit_push_left(vm, v, (wabi_vector_digit) d, s);
  }
  return (wabi_vector) wabi_vector_deep_push_left(vm, v, (wabi_vector_deep) d, s);
}


wabi_vector
wabi_vector_push_left(wabi_vm vm,
                     wabi_val v,
                      wabi_vector d)
{
  return (wabi_vector) wabi_vector_push_left_generic(vm, v, d, 1L);
}


wabi_val
wabi_vector_left(wabi_vm vm, wabi_vector d)
{
  wabi_vector_digit l;
  wabi_word* t;

  switch(WABI_TAG(d)) {
  case wabi_tag_vector_deep:
    l = wabi_vector_deep_left((wabi_vector_deep) d);
    t = wabi_vector_digit_table(l);
    return (wabi_val) *t;

  case wabi_tag_vector_digit:
    if(wabi_vector_size((wabi_vector) d) == 0)
      return vm->nil;

    t = wabi_vector_digit_table((wabi_vector_digit) d);
    return (wabi_val) *t;
  default:
    return vm->nil;
  }
}

wabi_val
wabi_vector_right(wabi_vm vm, wabi_vector d)
{
  wabi_vector_digit r;
  wabi_size n;
  wabi_word* t;

  switch(WABI_TAG(d)) {
  case wabi_tag_vector_deep:
    r = wabi_vector_deep_right((wabi_vector_deep) d);
    n = wabi_vector_digit_node_size(r);
    t = wabi_vector_digit_table(r);

    return (wabi_val) *(t + n - 1);

  case wabi_tag_vector_digit:
    if(wabi_vector_size((wabi_vector) d) == 0)
      return vm->nil;

    t = wabi_vector_digit_table((wabi_vector_digit) d);
    n = wabi_vector_digit_node_size((wabi_vector_digit) d);
    return (wabi_val) *(t + n - 1);
  default:
    return vm->nil;
  }
}


static wabi_vector
wabi_vector_pop_left_generic(wabi_vm vm, wabi_vector d, wabi_size s);


static inline wabi_vector
wabi_vector_digit_pop_left(wabi_vm vm, wabi_vector_digit d0, wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t0, t;
  wabi_vector_digit d;

  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  d = wabi_vector_digit_new(vm, n0 - 1, s0 - s);
  if(! d) return NULL;

  t0 = wabi_vector_digit_table(d0);
  t = wabi_vector_digit_table(d);
  wordcopy(t, t0 + 1, n0 - 1);
  return (wabi_vector) d;
}


static inline wabi_vector
wabi_vector_deep_pop_left(wabi_vm vm, wabi_vector_deep d0, wabi_size s)
{
  wabi_vector_digit l0, r0, l;
  wabi_vector m0, m;
  wabi_size nl0;

  l0 = wabi_vector_deep_left(d0);
  m0 = wabi_vector_deep_middle(d0);
  r0 = wabi_vector_deep_right(d0);

  nl0 = wabi_vector_digit_node_size(l0);
  if(nl0 > 1) {
    l = (wabi_vector_digit) wabi_vector_digit_pop_left(vm, l0, s);
    if(! l) return NULL;
    return (wabi_vector) wabi_vector_deep_new(vm, l, m0, r0);
  }
  if(wabi_vector_is_empty((wabi_val) m)) {
    return (wabi_vector) r0;
  }
  l = (wabi_vector_digit) wabi_vector_left(vm, m0);
  if(! l) return NULL;

  m = wabi_vector_pop_left_generic(vm, m, wabi_vector_size((wabi_vector) l));
  if(! m) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l, m, r0);
}


static wabi_vector
wabi_vector_pop_left_generic(wabi_vm vm, wabi_vector d0, wabi_size s)
{
  switch(WABI_TAG(d0)) {
  case wabi_tag_vector_deep:
    return wabi_vector_deep_pop_left(vm, (wabi_vector_deep) d0, s);

  case wabi_tag_vector_digit:
    return wabi_vector_digit_pop_left(vm, (wabi_vector_digit) d0, s);
  default:
    return NULL;
  }
}


wabi_val
wabi_vector_pop_left(wabi_vm vm, wabi_vector d)
{
  if(wabi_vector_size(d) == 0)
    return vm->nil;

  return (wabi_val) wabi_vector_pop_left_generic(vm, d, 1L);
}


static wabi_vector
wabi_vector_pop_right_generic(wabi_vm vm, wabi_vector d, wabi_size s);


static inline wabi_vector
wabi_vector_digit_pop_right(wabi_vm vm, wabi_vector_digit d0, wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t0, t;
  wabi_vector_digit d;

  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  d = wabi_vector_digit_new(vm, n0 - 1, s0 - s);
  if(! d) return NULL;

  t0 = wabi_vector_digit_table(d0);
  t = wabi_vector_digit_table(d);
  wordcopy(t, t0, n0 - 1);
  return (wabi_vector) d;
}


static inline wabi_vector
wabi_vector_deep_pop_right(wabi_vm vm, wabi_vector_deep d0, wabi_size s)
{
  wabi_vector_digit l0, r0, r;
  wabi_vector m0, m;
  wabi_size nr0;

  l0 = wabi_vector_deep_left(d0);
  m0 = wabi_vector_deep_middle(d0);
  r0 = wabi_vector_deep_right(d0);

  nr0 = wabi_vector_digit_node_size(r0);
  if(nr0 > 1) {
    r = (wabi_vector_digit) wabi_vector_digit_pop_right(vm, r0, s);
    if(! r) return NULL;
    return (wabi_vector) wabi_vector_deep_new(vm, l0, m0, r);
  }
  if(wabi_vector_is_empty((wabi_val) m)) {
    return (wabi_vector) l0;
  }
  r = (wabi_vector_digit) wabi_vector_right(vm, m0);
  if(! r) return NULL;

  m = wabi_vector_pop_right_generic(vm, m, wabi_vector_size((wabi_vector) r));
  if(! m) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l0, m, r);
}


static wabi_vector
wabi_vector_pop_right_generic(wabi_vm vm, wabi_vector d0, wabi_size s)
{
  switch(WABI_TAG(d0)) {
  case wabi_tag_vector_deep:
    return wabi_vector_deep_pop_right(vm, (wabi_vector_deep) d0, s);

  case wabi_tag_vector_digit:
    return wabi_vector_digit_pop_right(vm, (wabi_vector_digit) d0, s);
  default:
    return NULL;
  }
}


wabi_val
wabi_vector_pop_right(wabi_vm vm, wabi_vector d)
{
  if(wabi_vector_size(d) == 0)
    return vm->nil;

  return (wabi_val) wabi_vector_pop_right_generic(vm, d, 1L);
}


static inline int
wabi_vector_is_digit(wabi_val v)
{
  return WABI_IS(wabi_tag_vector_digit, v);
}

wabi_vector
wabi_vector_concat(wabi_vm vm, wabi_vector l, wabi_vector r)
{
  wabi_vector_digit l1, r1, l2, r2;
  wabi_vector m1, m2, m;
  if(wabi_vector_is_empty((wabi_val) l)) {
    // [] . r
    return r;
  }
  if(wabi_vector_is_empty((wabi_val) r)) {
    // l . []
    return l;
  }
  if(wabi_vector_is_digit((wabi_val) l) && wabi_vector_is_digit((wabi_val) r)) {
    // TODO: optimize the case of nsize(l) + nsize(r) <=32
    // l . r
    m1 = (wabi_vector) wabi_vector_empty_new(vm);
    if(! m1) return NULL;

    return (wabi_vector) wabi_vector_deep_new(vm, (wabi_vector_digit) l, m1, (wabi_vector_digit) r);
  }
  if(wabi_vector_is_digit((wabi_val) l)) {
    // l . l2 m2 r2
    l2 = wabi_vector_deep_left((wabi_vector_deep) r);
    m2 = wabi_vector_deep_middle((wabi_vector_deep) r);
    r2 = wabi_vector_deep_right((wabi_vector_deep) r);
    m2 = (wabi_vector) wabi_vector_push_left_generic(vm, (wabi_val) l2, m2, wabi_vector_size((wabi_vector) l2));
    if(! m2) return NULL;

    return (wabi_vector) wabi_vector_deep_new(vm, (wabi_vector_digit) l, m2, r2);
  }
  if(wabi_vector_is_digit((wabi_val) r)) {
    // l1 m1 r1 . r
    l1 = wabi_vector_deep_left((wabi_vector_deep) l);
    m1 = wabi_vector_deep_middle((wabi_vector_deep) l);
    r1 = wabi_vector_deep_right((wabi_vector_deep) l);
    m1 = (wabi_vector) wabi_vector_push_right_generic(vm, m1, (wabi_val) r1, wabi_vector_size((wabi_vector) r1));
    if(! m1) return NULL;

    return (wabi_vector) wabi_vector_deep_new(vm, l1, m1, (wabi_vector_digit) r);
  }
    // TODO: optimize the case of nsize(r1) + nsize(l2) <=32
  // l1 m1 r1 . l2 m2 r2

  l1 = wabi_vector_deep_left((wabi_vector_deep) l);
  m1 = wabi_vector_deep_middle((wabi_vector_deep) l);
  r1 = wabi_vector_deep_right((wabi_vector_deep) l);

  l2 = wabi_vector_deep_left((wabi_vector_deep) r);
  m2 = wabi_vector_deep_middle((wabi_vector_deep) r);
  r2 = wabi_vector_deep_right((wabi_vector_deep) r);

  m1 = wabi_vector_push_right_generic(vm, m1, (wabi_val) r1, wabi_vector_size((wabi_vector) r1));
  if(! m1) return NULL;

  m2 = wabi_vector_push_left_generic(vm, (wabi_val) l2, m2, wabi_vector_size((wabi_vector) l2));
  if(! m2) return NULL;

  m = wabi_vector_concat(vm, m1, m2);
  if(! m) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l1, m, r2);
}


static wabi_val
wabi_vector_ref_generic(wabi_vector d, wabi_size p, wabi_size lvl);


static inline wabi_val
wabi_vector_ref_digit(wabi_vector_digit d, wabi_size p, wabi_size lvl)
{
  wabi_size n, s, j, x;
  wabi_val t;
  n = wabi_vector_digit_node_size(d);
  s = wabi_vector_size((wabi_vector) d);
  t = wabi_vector_digit_table(d);
  if(lvl == 0)
    return (wabi_val) *(t + p);

  x = 0;
  for (j = 0; j < n; j++) {
    d = (wabi_vector_digit) *(t + j);
    s = wabi_vector_size((wabi_vector) d);
    if(p < x + s) {
      return wabi_vector_ref_digit(d, p - x, lvl - 1);
    }
    x+= s;
  }
  return NULL;
}


static inline wabi_val
wabi_vector_ref_deep(wabi_vector_deep d, wabi_size p, wabi_size lvl)
{
  wabi_size sl, sm;
  wabi_vector_digit l, r;
  wabi_vector m;

  l = wabi_vector_deep_left(d);
  m = wabi_vector_deep_middle(d);
  r = wabi_vector_deep_right(d);

  sl = wabi_vector_size((wabi_vector) l);
  sm = wabi_vector_size(m);

  if(p < sl)
    return wabi_vector_ref_digit(l, p, lvl);

  if(p < sl + sm)
    return wabi_vector_ref_generic(m, p - sl, lvl + 1);

  return wabi_vector_ref_digit(r, p - sl - sm, lvl);
}


static wabi_val
wabi_vector_ref_generic(wabi_vector d, wabi_size p, wabi_size lvl)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_vector_digit:
    return wabi_vector_ref_digit((wabi_vector_digit) d, p, lvl);
  default:
    return wabi_vector_ref_deep((wabi_vector_deep) d, p, lvl);
  }
}


wabi_val
wabi_vector_ref(wabi_vector d, wabi_size p)
{
  return wabi_vector_ref_generic(d, p, 0L);
}


// SET


static wabi_vector
wabi_vector_set_generic(wabi_vm vm, wabi_vector d, wabi_size p, wabi_val v, wabi_size lvl);


static inline wabi_vector_digit
wabi_vector_set_digit(wabi_vm vm, wabi_vector_digit d0, wabi_size p, wabi_val v, wabi_size lvl)
{
  wabi_size n, s, j, x;
  wabi_vector_digit d, d1;
  wabi_val t;

  n = wabi_vector_digit_node_size(d0);
  d = (wabi_vector_digit) wabi_vm_alloc(vm, WABI_VECTOR_DIGIT_SIZE + n);
  if(! d)
    return NULL;

  wordcopy((wabi_word*) d, (wabi_word*) d0, WABI_VECTOR_DIGIT_SIZE + n);
  t = wabi_vector_digit_table(d);

  if(lvl == 0) {
    *(t + p) = (wabi_word) v;
    return d;
  }

  x = 0;
  for(j = 0; j < n; j ++) {
    d1 = (wabi_vector_digit) *(t + j);
    s = wabi_vector_size((wabi_vector) d1);
    if (p < x + s) {
      d1 = wabi_vector_set_digit(vm, d1, p - x, v, lvl - 1);
      if(! d1) return NULL;
      *(t + j) = (wabi_word) d1;
      return d;
    }
    x += s;
  }
  return NULL;
}


static inline wabi_vector_deep
wabi_vector_set_deep(wabi_vm vm, wabi_vector_deep d0, wabi_size p, wabi_val v, wabi_size lvl)
{

  wabi_size sl, sm;
  wabi_vector_digit l, r;
  wabi_vector m;
  wabi_vector_deep d;

  d = (wabi_vector_deep) wabi_vm_alloc(vm, WABI_VECTOR_DEEP_SIZE);
  if(! d)
    return NULL;

  wordcopy((wabi_word*) d, (wabi_word*) d0, WABI_VECTOR_DEEP_SIZE);

  l = wabi_vector_deep_left(d);
  m = wabi_vector_deep_middle(d);
  r = wabi_vector_deep_right(d);

  sl = wabi_vector_size((wabi_vector) l);
  sm = wabi_vector_size(m);

  if(p < sl) {
    l = wabi_vector_set_digit(vm, l, p, v, lvl);
    if(! l) return NULL;
    d->left = (wabi_word) l;
    return d;
  }
  if(p < sl + sm) {
    m = wabi_vector_set_generic(vm, m, p - sl, v, lvl + 1);
    if(! m) return NULL;
    d->middle = (wabi_word) m;
    return d;
  }

  r = wabi_vector_set_digit(vm, r, p - sl - sm, v, lvl);
  if(! r) return NULL;
  d->right = (wabi_word) r;
  return d;
}


static wabi_vector
wabi_vector_set_generic(wabi_vm vm, wabi_vector d, wabi_size p, wabi_val v, wabi_size lvl)
{
  if(WABI_IS(wabi_tag_vector_deep, d))
    return (wabi_vector) wabi_vector_set_deep(vm, (wabi_vector_deep) d, p, v, lvl);

  return (wabi_vector) wabi_vector_set_digit(vm, (wabi_vector_digit) d, p, v, lvl);
}
// END SET


static inline wabi_vector
wabi_vector_set(wabi_vm vm, wabi_vector d, wabi_size p, wabi_val v)
{
  return wabi_vector_set_generic(vm,d, p, v, 0l);
}






/* void */
/* wabi_vector_iterator_init(wabi_vector_iter iter, */
/*                          wabi_vector d) */
/* { */

/*   wabi_vector_iter_frame frame; */

/*   frame = iter->stack; */
/*   iter->top = 0; */
/*   frame->vector = d; */
/*   frame->pos = 0; */

/*   if(wabi_vector_size(d) == 0) { */
/*     iter->top--; */
/*     return; */
/*   } */
/*   if(WABI_IS(wabi_tag_vector_deep, d)) { */
/*     frame->pos++; */
/*     iter->top++; */
/*     frame = iter->stack + 1; */
/*     frame->vector = (wabi_vector) wabi_vector_deep_left((wabi_vector_deep) d); */
/*     frame->pos = 0; */
/*   } */
/* } */


/* wabi_val */
/* wabi_vector_iterator_current(wabi_vector_iter iter) */
/* { */
/*   wabi_vector_iter_frame frame; */
/*   wabi_vector_digit d; */
/*   wabi_val t; */

/*   if(iter->top < 0) return NULL; */

/*   frame = iter->stack + iter->top; */
/*   d = (wabi_vector_digit) frame->vector; */
/*   t = wabi_vector_digit_table(d); */
/*   return (wabi_val) *(t + frame->pos); */
/* } */

/* void */
/* wabi_vector_iterator_next(wabi_vector_iter iter) */
/* { */

/*   wabi_vector_iter_frame frame; */
/*   wabi_vector_digit d; */
/*   wabi_val t; */

/*   if(iter->top < 0) return ; */

/*   frame = iter->stack + iter->top; */
/*   d = (wabi_vector_digit) frame->vector; */
/*   frame->pos++; */
/*   if(frame->pos < wabi_vector_digit_node_size(d)) */
/*     return; */

/*   iter->top--; */
/*   if(iter->top < 0) return; */

/*   frame = iter->stack + iter->top; */
/*   if(WABI_IS(wabi_tag_vector_deep, frame->vector)) { */
/*     frame->vector = (wabi_vector) wabi_vector_deep_right((wabi_vector_deep) frame->vector); */
/*     frame->pos = 0; */
/*   } */
/* } */



/* int */
/* wabi_vector_cmp(wabi_vector left, wabi_vector right) */
/* { */
/*   wabi_vector_iter_t left_iter, right_iter; */
/*   wabi_val left_val, right_val; */
/*   int cmp; */

/*   do { */
/*     left_val = wabi_vector_iterator_current(&left_iter); */
/*     right_val = wabi_vector_iterator_current(&right_iter); */

/*     if(!left_val && !right_val) { */
/*       return 0; */
/*     } */
/*     else if(!right_val) { */
/*       return 1; */
/*     } */
/*     else if(!left_val) { */
/*       return -1; */
/*     } */
/*     else { */
/*       cmp = wabi_cmp(left_val, right_val); */
/*       if(cmp) return cmp; */
/*     } */
/*     wabi_vector_iterator_next(&left_iter); */
/*     wabi_vector_iterator_next(&right_iter); */
/*   } while(1); */
/* } */


static wabi_error_type
wabi_vector_vec_concat(wabi_vm vm)
{
  wabi_val ctrl, d;
  wabi_vector r;
  ctrl = vm->ctrl;
  r = (wabi_vector) wabi_vector_empty_new(vm);
  if(! r) return wabi_error_nomem;

  while(wabi_is_pair(ctrl)) {
    d = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_vector(d))
      return wabi_error_type_mismatch;

    r = wabi_vector_concat(vm, r, (wabi_vector) d);
    if(! r)
      return wabi_error_nomem;
  }
  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  vm->ctrl = (wabi_val) r;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec(wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  res = (wabi_vector) wabi_vector_empty_new(vm);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
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
wabi_vector_vec_len(wabi_vm vm)
{
  wabi_vector d;
  wabi_val r, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl))
    return wabi_error_bindings;
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  r = (wabi_val) wabi_fixnum_new(vm, wabi_vector_size(d));
  if(! r) return wabi_error_nomem;

  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_emp_p(wabi_vm vm)
{
  wabi_val res, ctrl, vec;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    vec = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_vector_is_empty(vec)) {
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
wabi_vector_vec_p(wabi_vm vm)
{
  wabi_val res, ctrl, vec;
  res = (wabi_val) wabi_vm_alloc(vm, 1);
  if(! res) return wabi_error_nomem;

  ctrl = vm->ctrl;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    vec = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_vector(vec)) {
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
wabi_vector_vec_push_right(wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  res = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) res))
    return wabi_error_type_mismatch;

  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
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
wabi_vector_vec_push_left(wabi_vm vm)
{
  wabi_vector d;
  wabi_val ctrl, v;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  d = wabi_vector_push_left(vm, v, d);
  if(! d)
    return wabi_error_nomem;

  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_left(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  v = wabi_vector_left(vm, d);

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_right(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  v = wabi_vector_right(vm, d);

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_pop_left(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  v = (wabi_val) wabi_vector_pop_left(vm, d);
  if(! v)
    v = (wabi_val) vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_pop_right(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  v = (wabi_val) wabi_vector_pop_right(vm, d);
  if(! v)
    v = (wabi_val) vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_ref(wabi_vm vm)
{
  wabi_val res, ctrl;
  wabi_vector d;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  if(! wabi_is_pair((wabi_val) ctrl))
    return wabi_error_bindings;

  n = (wabi_fixnum) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(!WABI_IS(wabi_tag_fixnum, n))
    return wabi_error_type_mismatch;

  if(!wabi_is_nil(ctrl))
    return wabi_error_bindings;

  x = WABI_CAST_INT64(n);
  s = wabi_vector_size(d);

  if(x < 0 || x >= s) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
    return wabi_error_none;
  }
  res = wabi_vector_ref(d, x);
  if(! res)
    return wabi_error_other;

  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


static wabi_error_type
wabi_vector_vec_set(wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d, res;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl))
    return wabi_error_bindings;

  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) d))
    return wabi_error_type_mismatch;

  while(wabi_is_pair(ctrl)) {

    n = (wabi_fixnum)wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    if (!WABI_IS(wabi_tag_fixnum, n))
      return wabi_error_type_mismatch;

    if (!wabi_is_pair(ctrl))
      return wabi_error_bindings;

    v = wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    x = WABI_CAST_INT64(n);
    s = wabi_vector_size(d);

    if (x < 0 || x >= s) {
      vm->ctrl = vm->nil;
      vm->cont = (wabi_val)wabi_cont_next((wabi_cont)vm->cont);
      return wabi_error_none;
    }
    d = wabi_vector_set(vm, d, x, v);
    if (!d)
      return wabi_error_nomem;
  }
  if(! wabi_is_nil(ctrl))
    return wabi_error_bindings;

  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
  return wabi_error_none;
}


wabi_error_type
wabi_vector_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;
  res = WABI_DEFN(vm, env, "vec", "vec", wabi_vector_vec);
  if(res) return res;
  res = WABI_DEFN(vm, env, "vec-len", "vec-len", wabi_vector_vec_len);
  if(res) return res;
  res = WABI_DEFN(vm, env, "vec?", "vec?", wabi_vector_vec_p);
  if(res) return res;
  // it is really useful? `(= [] it)`
  res = WABI_DEFN(vm, env, "vec-emp?", "vec-emp?", wabi_vector_vec_emp_p);
  if(res) return res;
  res = WABI_DEFN(vm, env, "push-right", "push-right", wabi_vector_vec_push_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "push-left", "push-left", wabi_vector_vec_push_left);
  if(res) return res;
  res = WABI_DEFN(vm, env, "right", "right", wabi_vector_vec_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "left", "left", wabi_vector_vec_left);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pop-left", "pop-left", wabi_vector_vec_pop_left);
  if(res) return res;
  res = WABI_DEFN(vm, env, "pop-right", "pop-right", wabi_vector_vec_pop_right);
  if(res) return res;
  res = WABI_DEFN(vm, env, "vec-concat", "vec-concat", wabi_vector_vec_concat);
  if(res) return res;
  res = WABI_DEFN(vm, env, "vec-ref", "vec-ref", wabi_vector_vec_ref);
  if(res) return res;
  res = WABI_DEFN(vm, env, "vec-set", "vec-set", wabi_vector_vec_set);
  if(res) return res;
  /* res = WABI_DEFN(vm, env, "vec-sub", "vec-sub", wabi_vector_vec_sub); */
  /* if(res) return res; */
}
