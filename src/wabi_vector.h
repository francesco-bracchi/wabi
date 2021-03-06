#ifndef wabi_vector_h

#define wabi_vector_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

#define WABI_VECTOR_ITER_STACK_SIZE 10

typedef struct wabi_vector_digit_struct
{
  wabi_word size;
  wabi_word nsize;
} wabi_vector_digit_t;

typedef wabi_vector_digit_t* wabi_vector_digit;

typedef struct wabi_vector_deep_struct
{
  wabi_word size;
  wabi_word middle;
  wabi_word left;
  wabi_word right;
} wabi_vector_deep_t;

typedef wabi_vector_deep_t* wabi_vector_deep;

typedef union wabi_vector_struct {
  wabi_word size;
  wabi_vector_digit single;
  wabi_vector_deep deep;
} wabi_vector_t;

typedef wabi_vector_t* wabi_vector;

typedef struct wabi_vector_iter_frame_struct {
  wabi_vector vector;
  wabi_size level;
  wabi_size pos;
} wabi_vector_iter_frame_t;

typedef wabi_vector_iter_frame_t* wabi_vector_iter_frame;

typedef struct wabi_vector_iter_struct
{
  wabi_vector_iter_frame_t stack[10];
  wabi_vector_iter_frame top;
} wabi_vector_iter_t;

typedef wabi_vector_iter_t* wabi_vector_iter;

#define WABI_VECTOR_DIGIT_SIZE wabi_sizeof(wabi_vector_digit_t)
#define WABI_VECTOR_DEEP_SIZE wabi_sizeof(wabi_vector_deep_t)

static const wabi_size wabi_vector_digit_max_size = 32;


static inline wabi_vector_digit
wabi_vector_digit_new(wabi_vm vm, wabi_size nsize, wabi_size size)
{
  wabi_vector_digit res;
  res = (wabi_vector_digit) wabi_vm_alloc(vm, WABI_VECTOR_DIGIT_SIZE + nsize);
  if(res) {
    res->nsize = nsize;
    res->size = size;
    WABI_SET_TAG(res, wabi_tag_vector_digit);
  }
  return res;
}

static inline wabi_vector
wabi_vector_empty(const wabi_vm vm) {
  return (wabi_vector) wabi_vector_digit_new(vm, 0, 0);
}


static inline wabi_size
wabi_vector_size(wabi_vector d)
{
  return WABI_WORD_VAL(d->size);
}


static inline wabi_size
wabi_vector_digit_node_size(wabi_vector_digit d)
{
  return d->nsize;
}


static inline wabi_word*
wabi_vector_digit_table(wabi_vector_digit d)
{
  return ((wabi_word*) d) + WABI_VECTOR_DIGIT_SIZE;
}


static inline void
wabi_vector_digit_set(wabi_vector_digit d, wabi_size idx, wabi_val v)
{
  *(wabi_vector_digit_table(d) + idx) = (wabi_word) v;
}


static inline wabi_vector_deep
wabi_vector_deep_new(wabi_vm vm, wabi_vector_digit l, wabi_vector m, wabi_vector_digit r)
{
  wabi_vector_deep res;
  res = (wabi_vector_deep) wabi_vm_alloc(vm, WABI_VECTOR_DEEP_SIZE);
  if (res) {
    res->size = wabi_vector_size((wabi_vector) l)
      + wabi_vector_size((wabi_vector) m)
      + wabi_vector_size((wabi_vector) r);
    res->left = (wabi_word) l;
    res->middle = (wabi_word) m;
    res->right = (wabi_word) r;
    WABI_SET_TAG(res, wabi_tag_vector_deep);
  }
  return res;
}


static inline int
wabi_vector_is_empty(wabi_val v) {
  return WABI_IS(wabi_tag_vector_digit,v) && wabi_vector_size((wabi_vector) v) == 0L;
}


static inline int
wabi_is_vector(wabi_val val)
{
  switch(WABI_TAG(val)) {
  case wabi_tag_vector_digit:
  case wabi_tag_vector_deep:
    return 1;
  default:
    return 0;
  }
}


static inline wabi_vector_digit
wabi_vector_deep_left(wabi_vector_deep d)
{
  return (wabi_vector_digit) d->left;
}


static inline wabi_vector_digit
wabi_vector_deep_right(wabi_vector_deep d)
{
  return (wabi_vector_digit) d->right;
}


static inline wabi_vector
wabi_vector_deep_middle(wabi_vector_deep d)
{
  return (wabi_vector) d->middle;
}

wabi_vector
wabi_vector_push_left(wabi_vm vm, wabi_val v, wabi_vector d);


wabi_vector
wabi_vector_push_right(wabi_vm vm, wabi_vector d, wabi_val v);


wabi_val
wabi_vector_left(wabi_vm vm, wabi_vector d);


wabi_val
wabi_vector_right(wabi_vm vm, wabi_vector d);


wabi_val
wabi_vector_pop_left(wabi_vm vm, wabi_vector d);


wabi_val
wabi_vector_pop_right(wabi_vm vm, wabi_vector d);


wabi_vector
wabi_vector_concat(wabi_vm vm, wabi_vector l, wabi_vector r);

wabi_val
wabi_vector_ref(const wabi_vector d,
                const wabi_size p);

void
wabi_vector_iter_init(const wabi_vector_iter iter,
                      const wabi_vector v);

void
wabi_vector_iter_next(const wabi_vector_iter iter);


wabi_val
wabi_vector_iter_current(const wabi_vector_iter iter);


wabi_vector
wabi_vector_set(const wabi_vm vm,
                const wabi_vector d,
                const wabi_size p,
                const wabi_val v);

#endif
