#define wabi_vector_c

#include "wabi_vector.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_list.h"
#include "wabi_error.h"
#include "wabi_cont.h"
#include "wabi_builtin.h"
#include "wabi_atom.h"
#include "wabi_number.h"
#include "wabi_hash.h"


static inline int
wabi_vector_is_digit(const wabi_vector v)
{
  return WABI_IS(wabi_tag_vector_digit, v);
}

static inline int
wabi_vector_is_deep(const wabi_vector v)
{
  return WABI_IS(wabi_tag_vector_deep, v);
}


static inline wabi_vector_digit
wabi_vector_empty_new(const wabi_vm vm) {
  return wabi_vector_digit_new(vm, 0, 0);
}


static inline wabi_vector_digit
wabi_vector_single_new(const wabi_vm vm,
                       const wabi_val v,
                       const wabi_size s)
{
  wabi_vector_digit d;
  wabi_val t;

  d = wabi_vector_digit_new(vm, 1L, s);
  if(vm->ert) return NULL;
  t = wabi_vector_digit_table(d);
  *t = (wabi_word) v;
  return d;
}


static wabi_vector
wabi_vector_push_right_generic(const wabi_vm vm,
                               const wabi_vector d,
                               const wabi_val v,
                               const wabi_size s);


static inline wabi_vector
wabi_vector_digit_push_right(const wabi_vm vm,
                             const wabi_vector_digit d0,
                             const wabi_val v,
                             const wabi_size s)
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
    if(vm->ert) return NULL;
    t = wabi_vector_digit_table(d);
    wordcopy(t, t0, n0);
    *(t + n0) = (wabi_word)v;
    return (wabi_vector)d;
  }
  m = (wabi_vector)wabi_vector_empty_new(vm);
  if(vm->ert) return NULL;
  r = wabi_vector_single_new(vm, v, s);
  if(vm->ert) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, d0, (wabi_vector) m, r);
}


static inline wabi_vector_deep
wabi_vector_deep_push_right(const wabi_vm vm,
                            const wabi_vector_deep d0,
                            const wabi_val v,
                            const wabi_size s)
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
    if(vm->ert) return NULL;
    t = wabi_vector_digit_table(r);
    wordcopy(t, tr, nr);
    *(t + nr) = (wabi_word) v;
    return wabi_vector_deep_new(vm, l0, m0, r);
  }
  m = wabi_vector_push_right_generic(vm, m0, (wabi_val) r0, sr);
  if(vm->ert) return NULL;
  r = wabi_vector_single_new(vm, v, s);
  if(vm->ert) return NULL;
  return wabi_vector_deep_new(vm, l0, m, r);
}


static wabi_vector
wabi_vector_push_right_generic(const wabi_vm vm,
                               const wabi_vector d,
                               const wabi_val v,
                               const wabi_size s)
{
  if(wabi_vector_is_digit(d)) {
    return (wabi_vector) wabi_vector_digit_push_right(vm, (wabi_vector_digit) d, v, s);
  }
  return (wabi_vector) wabi_vector_deep_push_right(vm, (wabi_vector_deep) d, v, s);
}


wabi_vector
wabi_vector_push_right(const wabi_vm vm,
                      const wabi_vector d,
                      const wabi_val v)
{
  return (wabi_vector) wabi_vector_push_right_generic(vm, d, v, 1L);
}


static wabi_vector
wabi_vector_push_left_generic(const wabi_vm vm,
                              const wabi_val v,
                              const wabi_vector d,
                              const wabi_size s);


static inline wabi_vector
wabi_vector_digit_push_left(const wabi_vm vm,
                            const wabi_val v,
                            const wabi_vector_digit d0,
                            const wabi_size s)
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
    if(vm->ert) return NULL;
    t = wabi_vector_digit_table(d);
    *t = (wabi_word)v;
    wordcopy(t + 1, t0, n0);
    return (wabi_vector)d;
  }
  m = (wabi_vector)wabi_vector_empty_new(vm);
  if(vm->ert) return NULL;
  l = wabi_vector_single_new(vm, v, s);
  if(vm->ert) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l, (wabi_vector) m, d0);
}


static inline wabi_vector_deep
wabi_vector_deep_push_left(const wabi_vm vm,
                           const wabi_val v,
                           const wabi_vector_deep d0,
                           const wabi_size s)
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
    if(vm->ert) return NULL;
    t = wabi_vector_digit_table(l);
    *t = (wabi_word)v;
    wordcopy(t + 1, tl, nl);
    return wabi_vector_deep_new(vm, l, m0, r0);
  }
  m = wabi_vector_push_left_generic(vm, (wabi_val) l0, m0, sl);
  if(vm->ert) return NULL;
  l = wabi_vector_single_new(vm, v, s);
  if(vm->ert) return NULL;
  return wabi_vector_deep_new(vm, l, m, r0);
}


static wabi_vector
wabi_vector_push_left_generic(const wabi_vm vm,
                              const wabi_val v,
                              const wabi_vector d,
                              const wabi_size s)
{
  if(wabi_vector_is_digit(d)) {
    return (wabi_vector) wabi_vector_digit_push_left(vm, v, (wabi_vector_digit) d, s);
  }
  return (wabi_vector) wabi_vector_deep_push_left(vm, v, (wabi_vector_deep) d, s);
}


wabi_vector
wabi_vector_push_left(const wabi_vm vm,
                      const wabi_val v,
                      const wabi_vector d)
{
  return (wabi_vector) wabi_vector_push_left_generic(vm, v, d, 1L);
}


wabi_val
wabi_vector_left(const wabi_vm vm, const wabi_vector d)
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
wabi_vector_right(const wabi_vm vm, const wabi_vector d)
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
wabi_vector_pop_left_generic(const wabi_vm vm,
                             const wabi_vector d,
                             const wabi_size s);


static inline wabi_vector
wabi_vector_digit_pop_left(const wabi_vm vm,
                           const wabi_vector_digit d0,
                           const wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t0, t;
  wabi_vector_digit d;

  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  d = wabi_vector_digit_new(vm, n0 - 1, s0 - s);
  if(vm->ert) return NULL;

  t0 = wabi_vector_digit_table(d0);
  t = wabi_vector_digit_table(d);
  wordcopy(t, t0 + 1, n0 - 1);
  return (wabi_vector) d;
}


static inline wabi_vector
wabi_vector_deep_pop_left(const wabi_vm vm,
                          const wabi_vector_deep d0,
                          const wabi_size s)
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
    if(vm->ert) return NULL;
    return (wabi_vector) wabi_vector_deep_new(vm, l, m0, r0);
  }
  if(wabi_vector_is_empty((wabi_val) m0)) {
    return (wabi_vector) r0;
  }
  l = (wabi_vector_digit) wabi_vector_left(vm, m0);
  if(vm->ert) return NULL;
  m = wabi_vector_pop_left_generic(vm, m0, wabi_vector_size((wabi_vector) l));
  if(vm->ert) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l, m, r0);
}


static wabi_vector
wabi_vector_pop_left_generic(const wabi_vm vm,
                             const wabi_vector d0,
                             const wabi_size s)
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
wabi_vector_pop_left(const wabi_vm vm, const wabi_vector d)
{
  if(wabi_vector_size(d) == 0)
    return vm->nil;

  return (wabi_val) wabi_vector_pop_left_generic(vm, d, 1L);
}


static wabi_vector
wabi_vector_pop_right_generic(const wabi_vm vm,
                              const wabi_vector d,
                              const wabi_size s);


static inline wabi_vector
wabi_vector_digit_pop_right(const wabi_vm vm,
                            const wabi_vector_digit d0,
                            const wabi_size s)
{
  wabi_size s0, n0;
  wabi_val t0, t;
  wabi_vector_digit d;

  n0 = wabi_vector_digit_node_size(d0);
  s0 = wabi_vector_size((wabi_vector) d0);
  d = wabi_vector_digit_new(vm, n0 - 1, s0 - s);
  if(vm->ert) return NULL;

  t0 = wabi_vector_digit_table(d0);
  t = wabi_vector_digit_table(d);
  wordcopy(t, t0, n0 - 1);
  return (wabi_vector) d;
}


static inline wabi_vector
wabi_vector_deep_pop_right(const wabi_vm vm,
                           const wabi_vector_deep d0,
                           const wabi_size s)
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
    if(vm->ert) return NULL;
    return (wabi_vector) wabi_vector_deep_new(vm, l0, m0, r);
  }
  if(wabi_vector_is_empty((wabi_val) m0)) {
    return (wabi_vector) l0;
  }
  r = (wabi_vector_digit) wabi_vector_right(vm, m0);
  if(vm->ert) return NULL;
  m = wabi_vector_pop_right_generic(vm, m0, wabi_vector_size((wabi_vector) r));
  if(vm->ert) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l0, m, r);
}


static wabi_vector
wabi_vector_pop_right_generic(const wabi_vm vm,
                              const wabi_vector d0,
                              const wabi_size s)
{
  if(wabi_vector_is_digit(d0)) {
    return wabi_vector_digit_pop_right(vm, (wabi_vector_digit) d0, s);
  }
  return wabi_vector_deep_pop_right(vm, (wabi_vector_deep) d0, s);
}


wabi_val
wabi_vector_pop_right(const wabi_vm vm,
                      const wabi_vector d)
{
  if(wabi_vector_size(d) == 0)
    return vm->nil;

  return (wabi_val) wabi_vector_pop_right_generic(vm, d, 1L);
}


wabi_vector
wabi_vector_concat(const wabi_vm vm,
                   const wabi_vector l,
                   const wabi_vector r)
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
  if(wabi_vector_is_digit(l) && wabi_vector_is_digit(r)) {
    // TODO: optimize the case of nsize(l) + nsize(r) <=32
    // l . r
    m1 = (wabi_vector) wabi_vector_empty_new(vm);
    if(vm->ert) return NULL;

    return (wabi_vector) wabi_vector_deep_new(vm, (wabi_vector_digit) l, m1, (wabi_vector_digit) r);
  }
  if(wabi_vector_is_digit(l)) {
    // l . l2 m2 r2
    l2 = wabi_vector_deep_left((wabi_vector_deep) r);
    m2 = wabi_vector_deep_middle((wabi_vector_deep) r);
    r2 = wabi_vector_deep_right((wabi_vector_deep) r);
    m2 = (wabi_vector) wabi_vector_push_left_generic(vm, (wabi_val) l2, m2, wabi_vector_size((wabi_vector) l2));
    if(vm->ert) return NULL;

    return (wabi_vector) wabi_vector_deep_new(vm, (wabi_vector_digit) l, m2, r2);
  }
  if(wabi_vector_is_digit(r)) {
    // l1 m1 r1 . r
    l1 = wabi_vector_deep_left((wabi_vector_deep) l);
    m1 = wabi_vector_deep_middle((wabi_vector_deep) l);
    r1 = wabi_vector_deep_right((wabi_vector_deep) l);
    m1 = (wabi_vector) wabi_vector_push_right_generic(vm, m1, (wabi_val) r1, wabi_vector_size((wabi_vector) r1));
    if(vm->ert) return NULL;

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
  if(vm->ert) return NULL;
  m2 = wabi_vector_push_left_generic(vm, (wabi_val) l2, m2, wabi_vector_size((wabi_vector) l2));
  if(vm->ert) return NULL;
  m = wabi_vector_concat(vm, m1, m2);
  if(vm->ert) return NULL;

  return (wabi_vector) wabi_vector_deep_new(vm, l1, m, r2);
}


static wabi_val
wabi_vector_ref_generic(const wabi_vector d,
                        const wabi_size p,
                        const wabi_size lvl);


static inline wabi_val
wabi_vector_ref_digit(const wabi_vector_digit d,
                      const wabi_size p,
                      const wabi_size lvl)
{
  wabi_size n, s, j, x;
  wabi_val t;
  wabi_vector_digit d1;

  n = wabi_vector_digit_node_size(d);
  s = wabi_vector_size((wabi_vector) d);
  t = wabi_vector_digit_table(d);
  if(lvl == 0)
    return (wabi_val) *(t + p);

  x = 0;
  for (j = 0; j < n; j++) {
    d1 = (wabi_vector_digit) *(t + j);
    s = wabi_vector_size((wabi_vector) d1);
    if(p < x + s) {
      return wabi_vector_ref_digit(d1, p - x, lvl - 1);
    }
    x+= s;
  }
  return NULL;
}


static inline wabi_val
wabi_vector_ref_deep(const wabi_vector_deep d,
                     const wabi_size p,
                     const wabi_size lvl)
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
wabi_vector_ref_generic(const wabi_vector d,
                        const wabi_size p,
                        const wabi_size lvl)
{
  if(wabi_vector_is_digit(d)) {
    return wabi_vector_ref_digit((wabi_vector_digit) d, p, lvl);
  }
  return wabi_vector_ref_deep((wabi_vector_deep) d, p, lvl);
}


wabi_val
wabi_vector_ref(const wabi_vector d,
                const wabi_size p)
{
  return wabi_vector_ref_generic(d, p, 0L);
}


// SET


static wabi_vector
wabi_vector_set_generic(const wabi_vm vm,
                        const wabi_vector d,
                        const wabi_size p,
                        const wabi_val v,
                        const wabi_size lvl);


static inline wabi_vector_digit
wabi_vector_set_digit(const wabi_vm vm,
                      const wabi_vector_digit d0,
                      const wabi_size p,
                      const wabi_val v,
                      const wabi_size lvl)
{
  wabi_size n, s, j, x;
  wabi_vector_digit d, d1;
  wabi_val t;

  n = wabi_vector_digit_node_size(d0);
  d = (wabi_vector_digit) wabi_vm_alloc(vm, WABI_VECTOR_DIGIT_SIZE + n);
  if(vm->ert) return NULL;

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
      if(vm->ert) return NULL;
      *(t + j) = (wabi_word) d1;
      return d;
    }
    x += s;
  }
  return NULL;
}


static inline wabi_vector_deep
wabi_vector_set_deep(const wabi_vm vm,
                     const wabi_vector_deep d0,
                     const wabi_size p,
                     const wabi_val v,
                     const wabi_size lvl)
{

  wabi_size sl, sm;
  wabi_vector_digit l, r;
  wabi_vector m;
  wabi_vector_deep d;

  d = (wabi_vector_deep) wabi_vm_alloc(vm, WABI_VECTOR_DEEP_SIZE);
  if(vm->ert) return NULL;

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
    if(vm->ert) return NULL;
    d->middle = (wabi_word) m;
    return d;
  }

  r = wabi_vector_set_digit(vm, r, p - sl - sm, v, lvl);
  if(vm->ert) return NULL;
  d->right = (wabi_word) r;
  return d;
}


static wabi_vector
wabi_vector_set_generic(const wabi_vm vm,
                        const wabi_vector d,
                        const wabi_size p,
                        const wabi_val v,
                        const wabi_size lvl)
{
  if(wabi_vector_is_deep(d))
    return (wabi_vector) wabi_vector_set_deep(vm, (wabi_vector_deep) d, p, v, lvl);

  return (wabi_vector) wabi_vector_set_digit(vm, (wabi_vector_digit) d, p, v, lvl);
}


static inline wabi_vector
wabi_vector_set(const wabi_vm vm,
                const wabi_vector d,
                const wabi_size p,
                const wabi_val v)
{
  return wabi_vector_set_generic(vm, d, p, v, 0l);
}


static void
wabi_vector_vec_concat(const wabi_vm vm)
{
  wabi_val ctrl, d;
  wabi_vector r;
  ctrl = vm->ctrl;
  r = (wabi_vector) wabi_vector_empty_new(vm);
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    d = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_vector(d)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    r = wabi_vector_concat(vm, r, (wabi_vector) d);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  vm->ctrl = (wabi_val) r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec(const wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  res = (wabi_vector) wabi_vector_empty_new(vm);
  if(vm->ert) return;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_len(const wabi_vm vm)
{
  wabi_vector d;
  wabi_val r, ctrl;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  r = (wabi_val) wabi_fixnum_new(vm, wabi_vector_size(d));
  if(vm->ert) return;

  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_p(const wabi_vm vm)
{
  wabi_val ctrl, vec;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    vec = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! wabi_is_vector(vec)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_push_right(const wabi_vm vm)
{
  wabi_vector res;
  wabi_val ctrl, a;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) res)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(wabi_is_pair(ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = wabi_vector_push_right(vm, res, a);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_push_left(const wabi_vm vm)
{
  wabi_vector d;
  wabi_val ctrl, v;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = wabi_vector_push_left(vm, v, d);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_left(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)){
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = wabi_vector_left(vm, d);
  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_right(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = wabi_vector_right(vm, d);

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_pop_left(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;

  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = (wabi_val) wabi_vector_pop_left(vm, d);
  if(!v) v = vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_pop_right(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  ctrl = vm->ctrl;

  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  v = (wabi_val) wabi_vector_pop_right(vm, d);
  if(! v) v = vm->nil;

  vm->ctrl = v;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_ref(const wabi_vm vm)
{
  wabi_val res, ctrl;
  wabi_vector d;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_vector((wabi_val) d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair((wabi_val) ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  n = (wabi_fixnum) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(!wabi_is_fixnum(n)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  x = WABI_CAST_INT64(n);
  s = wabi_vector_size(d);

  if(x < 0 || x >= s) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
    return;
  }
  res = wabi_vector_ref(d, x);
  if(! res) {
    vm->ert = wabi_error_other;
    return;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_vector_vec_set(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_vector d;
  wabi_fixnum n;
  wabi_size s, x;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  d = (wabi_vector) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if (!wabi_is_vector((wabi_val)d)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(wabi_is_pair(ctrl)) {

    n = (wabi_fixnum)wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    if (!wabi_is_fixnum(n)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    if (!wabi_is_pair(ctrl)) {
      vm->ert = wabi_error_bindings;
      return;
    }
    v = wabi_car((wabi_pair)ctrl);
    ctrl = wabi_cdr((wabi_pair)ctrl);

    x = WABI_CAST_INT64(n);
    s = wabi_vector_size(d);

    if (x < 0 || x >= s) {
      vm->ctrl = vm->nil;
      vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
      return;
    }
    d = wabi_vector_set(vm, d, x, v);
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = (wabi_val) d;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static inline void
wabi_vector_iter_move(const wabi_vector_iter iter)
{
  wabi_vector_iter_frame f, f0;
  wabi_size n0;
  wabi_vector v0;
  wabi_val t0;

  for(;;) {
    if(iter->top < iter->stack) return;
    f0 = iter->top;
    v0 = f0->vector;
    if(wabi_vector_is_digit(v0)) {
      n0 = wabi_vector_digit_node_size((wabi_vector_digit) v0);
      if (f0->pos >= n0) {
        iter->top--;
        if(iter->top < iter->stack) return;
        iter->top->pos++;
        continue;
      }
      if (f0->level > 0) {
        t0 = wabi_vector_digit_table((wabi_vector_digit) v0);
        iter->top++;
        f = iter->top;
        f->level = f0->level - 1;
        f->pos = 0;
        f->vector = (wabi_vector) * (t0 + f0->pos);
        continue;
      }
      return;
    }
    // deep
    if (f0->pos >= 3) {
      iter->top--;
      if(iter->top < iter->stack) return;
      iter->top->pos++;
      continue;
    }
    iter->top++;
    f = iter->top;
    f->pos = 0;

    if (f0->pos <= 0) {
      f->level = f0->level;
      f->vector = (wabi_vector) wabi_vector_deep_left((wabi_vector_deep) v0);
    }
    if (f0->pos == 1) {
      f->level = f0->level + 1;
      f->vector = (wabi_vector) wabi_vector_deep_middle((wabi_vector_deep) v0);
    }
    if (f0->pos == 2) {
      f->level = f0->level;
      f->vector = (wabi_vector) wabi_vector_deep_right((wabi_vector_deep) v0);
    }
  }
}


void
wabi_vector_iter_next(const wabi_vector_iter iter)
{
  iter->top->pos++;
  wabi_vector_iter_move(iter);
}


void
wabi_vector_iter_init(const wabi_vector_iter iter,
                      const wabi_vector v)
{
  wabi_vector_iter_frame f0;

  f0= iter->stack;
  iter->top = f0;

  f0->vector = v;
  f0->level = 0L;
  f0->pos = 0;
  wabi_vector_iter_move(iter);
}


wabi_val
wabi_vector_iter_current(const wabi_vector_iter iter)
{
  wabi_vector_iter_frame f;
  wabi_val t;

  if(iter->top < iter->stack) {
    return NULL;
  }

  f = iter->top;
  t = wabi_vector_digit_table((wabi_vector_digit) f->vector);
  return (wabi_val) *(t + f->pos);
}


void
wabi_vector_hash(const wabi_hash_state state,
                 const wabi_vector v)
{
  wabi_vector_iter_t iter;
  wabi_val x;

  wabi_vector_iter_init(&iter, v);
  wabi_hash_step(state, "V", 1);
  for(;;) {
    x = wabi_vector_iter_current(&iter);
    if(! x) return;
    wabi_hash_val(state, x);
    wabi_vector_iter_next(&iter);
  }
}

// TODO: rename push/pop left right
void
wabi_vector_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "vec", &wabi_vector_vec);
  if(vm->ert) return;
  wabi_defn(vm, env, "vec-len", &wabi_vector_vec_len);
  if(vm->ert) return;
  wabi_defn(vm, env, "vec?", &wabi_vector_vec_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "push-right", &wabi_vector_vec_push_right);
  if(vm->ert) return;
  wabi_defn(vm, env, "push-left", &wabi_vector_vec_push_left);
  if(vm->ert) return;
  wabi_defn(vm, env, "right", &wabi_vector_vec_right);
  if(vm->ert) return;
  wabi_defn(vm, env, "left", &wabi_vector_vec_left);
  if(vm->ert) return;
  wabi_defn(vm, env, "pop-left", &wabi_vector_vec_pop_left);
  if(vm->ert) return;
  wabi_defn(vm, env, "pop-right", &wabi_vector_vec_pop_right);
  if(vm->ert) return;
  wabi_defn(vm, env, "vec-concat", &wabi_vector_vec_concat);
  if(vm->ert) return;
  wabi_defn(vm, env, "vec-ref", &wabi_vector_vec_ref);
  if(vm->ert) return;
  wabi_defn(vm, env, "vec-set", &wabi_vector_vec_set);
  // if(vm->ert) return;
  /* wabi_defn(vm, env, "vec-sub", "vec-sub", wabi_vector_vec_sub); */
  /* if(vm->ert) return; */
}
