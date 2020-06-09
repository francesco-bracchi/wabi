#ifndef wabi_deque_h

#define wabi_deque_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef struct wabi_deque_digit_struct
{
  wabi_word size;
  wabi_word nsize;
} wabi_deque_digit_t;

typedef wabi_deque_digit_t* wabi_deque_digit;

typedef struct wabi_deque_deep_struct
{
  wabi_word size;
  wabi_word middle;
  wabi_word left;
  wabi_word right;
} wabi_deque_deep_t;

typedef wabi_deque_deep_t* wabi_deque_deep;

typedef union wabi_deque_struct {
  wabi_word size;
  wabi_deque_digit single;
  wabi_deque_deep deep;
} wabi_deque_t;

typedef wabi_deque_t* wabi_deque;

#define WABI_DEQUE_DIGIT_SIZE wabi_sizeof(wabi_deque_digit_t)
#define WABI_DEQUE_DEEP_SIZE wabi_sizeof(wabi_deque_deep_t)


static inline wabi_deque_digit
wabi_deque_digit_new(wabi_vm vm, wabi_size nsize, wabi_size size)
{

  wabi_deque_digit res;
  res = (wabi_deque_digit) wabi_vm_alloc(vm, WABI_DEQUE_DIGIT_SIZE + nsize);
  if(res) {
    res->nsize = nsize;
    res->size = size;
    WABI_SET_TAG(res, wabi_tag_deque_digit);
  }
  return res;
}


static inline wabi_size
wabi_deque_size(wabi_deque d)
{
  return WABI_WORD_VAL(d->size);
}


static inline wabi_size
wabi_deque_digit_node_size(wabi_deque_digit d)
{
  return d->nsize;
}


static inline wabi_word*
wabi_deque_digit_table(wabi_deque_digit d)
{
  return ((wabi_word*) d) + WABI_DEQUE_DIGIT_SIZE;
}


static inline void
wabi_deque_digit_set(wabi_deque_digit d, wabi_size idx, wabi_val v)
{
  *(wabi_deque_digit_table(d) + idx) = (wabi_word) v;
}


static inline wabi_deque_deep
wabi_deque_deep_new(wabi_vm vm, wabi_deque_digit l, wabi_deque m, wabi_deque_digit r)
{
  wabi_deque_deep res;
  res = (wabi_deque_deep) wabi_vm_alloc(vm, WABI_DEQUE_DEEP_SIZE);
  if (res) {
    res->size = wabi_deque_size((wabi_deque) l)
      + wabi_deque_size((wabi_deque) m)
      + wabi_deque_size((wabi_deque) r);
    res->left = (wabi_word) l;
    res->middle = (wabi_word) m;
    res->right = (wabi_word) r;
    WABI_SET_TAG(res, wabi_tag_deque_deep);
  }
  return res;
}


static inline int
wabi_deque_is_empty(wabi_val v) {
  return WABI_IS(wabi_tag_deque_digit,v) && wabi_deque_size((wabi_deque) v) == 0L;
}


static inline int
wabi_is_deque(wabi_val val)
{
  switch(WABI_TAG(val)) {
  case wabi_tag_deque_digit:
  case wabi_tag_deque_deep:
    return 1;
  default:
    return 0;
  }
}


static inline wabi_deque_digit
wabi_deque_deep_left(wabi_deque_deep d)
{
  return (wabi_deque_digit) d->left;
}


static inline wabi_deque_digit
wabi_deque_deep_right(wabi_deque_deep d)
{
  return (wabi_deque_digit) d->right;
}


static inline wabi_deque
wabi_deque_deep_middle(wabi_deque_deep d)
{
  return (wabi_deque) d->middle;
}


wabi_deque
wabi_deque_push_left(wabi_vm vm, wabi_val v, wabi_deque d);


wabi_deque
wabi_deque_push_right(wabi_vm vm, wabi_deque d, wabi_val v);


wabi_val
wabi_deque_left(wabi_vm vm, wabi_deque d);


wabi_val
wabi_deque_right(wabi_vm vm, wabi_deque d);


wabi_deque
wabi_deque_pop_left(wabi_vm vm, wabi_deque d);


wabi_deque
wabi_deque_pop_right(wabi_vm vm, wabi_deque d);


wabi_error_type
wabi_deque_builtins(wabi_vm vm, wabi_env env);


static inline void
wabi_deque_digit_copy_val(wabi_vm vm, wabi_deque_digit d)
{
  wabi_size n;
  n = wabi_deque_digit_node_size(d);
  wabi_copy_val_size(vm, (wabi_val) d, WABI_DEQUE_DIGIT_SIZE + n);
}


static inline void
wabi_deque_deep_copy_val(wabi_vm vm, wabi_deque_deep d)
{
  wabi_copy_val_size(vm, (wabi_val) d, WABI_DEQUE_DEEP_SIZE);
}



static inline void
wabi_deque_digit_collect_val(wabi_vm vm, wabi_deque_digit d)
{
  // TBD
  wabi_val t, u;
  wabi_size n;
  t = wabi_deque_digit_table(d);
  n = wabi_deque_digit_node_size(d);
  u = t + n;
  while(t < u) {
    *t = (wabi_word) wabi_copy_val(vm, (wabi_val) *t);
    t++;
  }
  vm->stor.scan+= WABI_DEQUE_DIGIT_SIZE + n;
}


static inline void
wabi_deque_deep_collect_val(wabi_vm vm, wabi_deque_deep d)
{
  wabi_collect_val_size(vm, (wabi_val) d, WABI_DEQUE_DEEP_SIZE);
}

#endif
