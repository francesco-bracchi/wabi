#ifndef wabi_deque_h

#define wabi_deque_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef struct wabi_deque_empty_struct
{
  wabi_word nothing;
} wabi_deque_empty_t;

typedef wabi_deque_empty_t* wabi_deque_empty;

typedef struct wabi_deque_single_struct
{
  wabi_word val;
} wabi_deque_single_t;

typedef wabi_deque_single_t* wabi_deque_single;

typedef struct wabi_deque_deep_struct
{
  wabi_word middle;
  wabi_word left;
  wabi_word right;
} wabi_deque_deep_t;

typedef wabi_deque_deep_t* wabi_deque_deep;

typedef struct wabi_deque_digit1_struct
{
  wabi_word a;
} wabi_deque_digit1_t;

typedef wabi_deque_digit1_t* wabi_deque_digit1;

typedef struct wabi_deque_digit2_struct
{
  wabi_word a;
  wabi_word b;
} wabi_deque_digit2_t;

typedef wabi_deque_digit2_t* wabi_deque_digit2;

typedef struct wabi_deque_digit3_struct
{
  wabi_word a;
  wabi_word b;
  wabi_word c;
} wabi_deque_digit3_t;

typedef wabi_deque_digit3_t* wabi_deque_digit3;

typedef struct wabi_deque_digit4_struct
{
  wabi_word a;
  wabi_word b;
  wabi_word c;
  wabi_word d;
} wabi_deque_digit4_t;

typedef wabi_deque_digit4_t* wabi_deque_digit4;

typedef struct wabi_deque_node2_struct
{
  wabi_word l;
  wabi_word r;
} wabi_deque_node2_t;

typedef wabi_deque_node2_t* wabi_deque_node2;

typedef struct wabi_deque_node3_struct
{
  wabi_word l;
  wabi_word m;
  wabi_word r;
} wabi_deque_node3_t;

typedef wabi_deque_node3_t* wabi_deque_node3;

typedef union wabi_deque_struct {
  wabi_deque_empty empty;
  wabi_deque_single single;
  wabi_deque_deep deep;
} wabi_deque_t;

typedef wabi_deque_t* wabi_deque;

typedef union wabi_deque_digit_struct {
  wabi_word empty;
  wabi_deque_single single;
  wabi_deque_deep deep;
} wabi_deque_digit_t;

typedef wabi_deque_digit_t* wabi_deque_digit;

typedef union wabi_deque_node_union {
  wabi_deque_node2_t node2;
  wabi_deque_node3_t node3;
} wabi_deque_node_t;

typedef wabi_deque_node_t* wabi_deque_node;

#define WABI_DEQUE_EMPTY_SIZE wabi_sizeof(wabi_deque_empty_t)
#define WABI_DEQUE_SINGLE_SIZE wabi_sizeof(wabi_deque_single_t)
#define WABI_DEQUE_DEEP_SIZE wabi_sizeof(wabi_deque_deep_t)

#define WABI_DEQUE_DIGIT1_SIZE wabi_sizeof(wabi_deque_digit1_t)
#define WABI_DEQUE_DIGIT2_SIZE wabi_sizeof(wabi_deque_digit2_t)
#define WABI_DEQUE_DIGIT3_SIZE wabi_sizeof(wabi_deque_digit3_t)
#define WABI_DEQUE_DIGIT4_SIZE wabi_sizeof(wabi_deque_digit4_t)

#define WABI_DEQUE_NODE2_SIZE wabi_sizeof(wabi_deque_node2_t)
#define WABI_DEQUE_NODE3_SIZE wabi_sizeof(wabi_deque_node3_t)

// Constructors
static inline wabi_deque_empty
wabi_deque_empty_new(wabi_vm vm)
{
  wabi_deque_empty res;
  res = (wabi_deque_empty) wabi_vm_alloc(vm, WABI_DEQUE_EMPTY_SIZE);
  if(res) res->nothing = wabi_tag_deque_empty;
  return res;
}

static inline wabi_deque_single
wabi_deque_single_new(wabi_vm vm, wabi_val val)
{
  wabi_deque_single res;
  res = (wabi_deque_single) wabi_vm_alloc(vm, WABI_DEQUE_SINGLE_SIZE);
  if (res) {
    res->val = (wabi_word) val;
    WABI_SET_TAG(res, wabi_tag_deque_single);
  }
  return res;
}

static inline wabi_deque_deep
wabi_deque_deep_new(wabi_vm vm, wabi_deque_digit l, wabi_deque m, wabi_deque_digit r)
{
  wabi_deque_deep res;
  res = (wabi_deque_deep) wabi_vm_alloc(vm, WABI_DEQUE_DEEP_SIZE);
  if (res) {
    res->left = (wabi_word) l;
    res->middle = (wabi_word) m;
    res->right = (wabi_word) r;
    WABI_SET_TAG(res, wabi_tag_deque_deep);
  }
  return res;
}

static inline wabi_deque_digit1
wabi_deque_digit1_new(wabi_vm vm, wabi_val a)
{
  wabi_deque_digit1 res;
  res = (wabi_deque_digit1) wabi_vm_alloc(vm, WABI_DEQUE_DIGIT1_SIZE);
  if (res) {
    res->a = (wabi_word) a;
    WABI_SET_TAG(res, wabi_tag_deque_digit1);
  }
  return res;
}

static inline wabi_deque_digit2
wabi_deque_digit2_new(wabi_vm vm, wabi_val a, wabi_val b)
{
  wabi_deque_digit2 res;
  res = (wabi_deque_digit2) wabi_vm_alloc(vm, WABI_DEQUE_DIGIT2_SIZE);
  if (res) {
    res->a = (wabi_word) a;
    res->b = (wabi_word) b;
    WABI_SET_TAG(res, wabi_tag_deque_digit2);
  }
  return res;
}

static inline wabi_deque_digit3
wabi_deque_digit3_new(wabi_vm vm, wabi_val a, wabi_val b, wabi_val c)
{
  wabi_deque_digit3 res;
  res = (wabi_deque_digit3) wabi_vm_alloc(vm, WABI_DEQUE_DIGIT3_SIZE);
  if (res) {
    res->a = (wabi_word) a;
    res->b = (wabi_word) b;
    res->c = (wabi_word) c;
    WABI_SET_TAG(res, wabi_tag_deque_digit3);
  }
  return res;
}

static inline wabi_deque_digit4
wabi_deque_digit4_new(wabi_vm vm, wabi_val a, wabi_val b, wabi_val c, wabi_val d)
{
  wabi_deque_digit4 res;
  res = (wabi_deque_digit4) wabi_vm_alloc(vm, WABI_DEQUE_DIGIT4_SIZE);
  if (res) {
    res->a = (wabi_word) a;
    res->b = (wabi_word) b;
    res->c = (wabi_word) c;
    res->d = (wabi_word) d;
    WABI_SET_TAG(res, wabi_tag_deque_digit4);
  }
  return res;
}

static inline wabi_deque_node2
wabi_deque_node2_new(wabi_vm vm, wabi_val l, wabi_val r)
{

  wabi_deque_node2 res;
  res = (wabi_deque_node2) wabi_vm_alloc(vm, WABI_DEQUE_NODE2_SIZE);
  if (res) {
    res->l = (wabi_word) l;
    res->r = (wabi_word) r;
    WABI_SET_TAG(res, wabi_tag_deque_node2);
  }
  return res;
}

static inline wabi_deque_node3
wabi_deque_node3_new(wabi_vm vm, wabi_val l, wabi_val m, wabi_val r)
{

  wabi_deque_node3 res;
  res = (wabi_deque_node3) wabi_vm_alloc(vm, WABI_DEQUE_NODE3_SIZE);
  if (res) {
    res->l = (wabi_word) l;
    res->m = (wabi_word) m;
    res->r = (wabi_word) r;
    WABI_SET_TAG(res, wabi_tag_deque_node3);
  }
  return res;
}

// Accessors

static inline wabi_val
wabi_deque_single_val(wabi_deque_single s)
{
  return (wabi_val) WABI_WORD_VAL(s->val);
}

static inline wabi_val
wabi_deque_deep_left(wabi_deque_deep d)
{
  return (wabi_val) d->left;
}

static inline wabi_val
wabi_deque_deep_middle(wabi_deque_deep d)
{
  return (wabi_val) WABI_WORD_VAL(d->middle);
}

static inline wabi_val
wabi_deque_deep_right(wabi_deque_deep d)
{
  return (wabi_val) d->right;
}

static inline wabi_val
wabi_deque_digit1_a(wabi_deque_digit1 d)
{
  return (wabi_val) WABI_WORD_VAL(d->a);
}

static inline wabi_val
wabi_deque_digit2_a(wabi_deque_digit2 d)
{
  return (wabi_val) WABI_WORD_VAL(d->a);
}

static inline wabi_val
wabi_deque_digit3_a(wabi_deque_digit3 d)
{
  return (wabi_val) WABI_WORD_VAL(d->a);
}

static inline wabi_val
wabi_deque_digit4_a(wabi_deque_digit4 d)
{
  return (wabi_val) WABI_WORD_VAL(d->a);
}

static inline wabi_val
wabi_deque_digit2_b(wabi_deque_digit2 d)
{
  return (wabi_val) d->b;
}

static inline wabi_val
wabi_deque_digit3_b(wabi_deque_digit3 d)
{
  return (wabi_val) d->b;
}

static inline wabi_val
wabi_deque_digit4_b(wabi_deque_digit4 d)
{
  return (wabi_val) d->b;
}

static inline wabi_val
wabi_deque_digit3_c(wabi_deque_digit3 d)
{
  return (wabi_val) d->c;
}

static inline wabi_val
wabi_deque_digit4_c(wabi_deque_digit4 d)
{
  return (wabi_val) d->c;
}

static inline wabi_val
wabi_deque_digit4_d(wabi_deque_digit4 d)
{
  return (wabi_val) d->d;
}

static inline wabi_val
wabi_deque_node2_l(wabi_deque_node2 n)
{
  return (wabi_val) WABI_WORD_VAL(n->l);
}

static inline wabi_val
wabi_deque_node2_r(wabi_deque_node2 n)
{
  return (wabi_val) n->r;
}

static inline wabi_val
wabi_deque_node3_l(wabi_deque_node3 n)
{
  return (wabi_val) WABI_WORD_VAL(n->l);
}

static inline wabi_val
wabi_deque_node3_m(wabi_deque_node3 n)
{
  return (wabi_val) n->m;
}

static inline wabi_val
wabi_deque_node3_r(wabi_deque_node3 n)
{
  return (wabi_val) n->r;
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
#endif
