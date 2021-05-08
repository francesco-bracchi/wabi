#define wabi_pr_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_pr.h"
#include "wabi_list.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_number.h"
#include "wabi_symbol.h"
#include "wabi_combiner.h"
#include "wabi_place.h"
#include "wabi_vector.h"
#include "wabi_atom.h"
/* #include "wabi_env.h" */

void
wabi_pr_binary(wabi_binary val);


void
wabi_pr_bin_leaf(wabi_binary_leaf leaf)
{
  char *from, *to, *c;
  from = (char*) leaf->data_ptr;
  to = from + WABI_WORD_VAL(leaf->length);

  for(c = from; c < to; c++)
    putchar(*c);
}


void
wabi_pr_bin_node(wabi_binary_node node)
{
  wabi_pr_binary((wabi_binary) node->left);
  wabi_pr_binary((wabi_binary) node->right);
}


void wabi_pr_binary(wabi_binary val)
{
  if(WABI_TAG(val) == wabi_tag_bin_leaf) {
    wabi_pr_bin_leaf((wabi_binary_leaf) val);
    return;
  }
  wabi_pr_bin_node((wabi_binary_node) val);
}


void wabi_pr_pair(const wabi_vm vm, wabi_pair val) {
  wabi_val car, cdr;

  do {
    car = wabi_car(val);
    cdr = wabi_cdr(val);

    if(wabi_is_pair(cdr)) {
      wabi_pr(vm, car);
      putchar(' ');
      val = (wabi_pair) cdr;
      continue;
    }

    if(wabi_atom_is_empty(vm, cdr)) {
      wabi_pr(vm, car);
      return;
    }

    wabi_pr(vm, car);
    printf(" . ");
    wabi_pr(vm, cdr);
    return;
  } while(1);
}


void
wabi_pr_map_entry(wabi_vm vm, wabi_map_entry entry)
{
  wabi_pr(vm, (wabi_val) entry->key);
  putchar(' ');
  wabi_pr(vm, (wabi_val) WABI_WORD_VAL(entry->value));
}


void
wabi_pr_map(wabi_vm vm, wabi_map map);


void
wabi_pr_map_array(wabi_vm vm, wabi_map_array map)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word size = WABI_MAP_ARRAY_SIZE(map);
  for(unsigned int j = 0; j < size; j++) {
    wabi_map_entry row = (wabi_map_entry)(table + j);
    wabi_pr_map_entry(vm, row);
    if(size - j - 1) {
      putchar(' ');
    }
  }
}


void
wabi_pr_map_hash(wabi_vm vm, wabi_map_hash map)
{
  wabi_word bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  wabi_word size = WABI_MAP_BITMAP_COUNT(bitmap);
  for(unsigned int j = 0; j < size; j++) {
    wabi_pr_map(vm, table + j);
    putchar(' ');
  }
}


void
wabi_pr_map(wabi_vm vm, wabi_map map)
{
  switch(WABI_TAG(map)) {
  case wabi_tag_map_array:
    wabi_pr_map_array(vm, (wabi_map_array) map);
    return;
  case wabi_tag_map_hash:
    wabi_pr_map_hash(vm, (wabi_map_hash) map);
    return;
  case wabi_tag_map_entry:
    wabi_pr_map_entry(vm, (wabi_map_entry) map);
    return;
  default:
    // printf("fox %lx\n", *((wabi_word*) map));
    return;
  }
}


void
wabi_pr_env(const wabi_vm vm, wabi_env env)
{
  printf("#env[%lx]", env->uid);
}


void
wabi_pr_applicative(wabi_vm vm, wabi_combiner_derived val)
{
  wabi_val body;

  printf("(fn ");
  wabi_pr(vm, (wabi_val) val->parameters);
  printf(" ");
  body = (wabi_val) val->body;
  while(WABI_IS(wabi_tag_pair, body)) {
    wabi_pr(vm, wabi_car((wabi_pair) body));
    body = wabi_cdr((wabi_pair) body);
    if(! wabi_atom_is_empty(vm, body)) {
      printf(" ");
    }
  }
  printf(")");
}


void
wabi_pr_operative(const wabi_vm vm, wabi_combiner_derived val)
{
  wabi_val body;

  printf("(fx ");
  wabi_pr(vm, (wabi_val) val->caller_env_name);
  printf(" ");
  wabi_pr(vm, (wabi_val) val->parameters);
  printf(" ");
  body = (wabi_val) val->body;
  while(wabi_is_pair(body)) {
    wabi_pr(vm, wabi_car((wabi_pair) body));
    body = wabi_cdr((wabi_pair) body);
    if(!wabi_atom_is_empty(vm, body)) {
      printf(" ");
    }
  }
  printf(")");
}


void
wabi_pr_binary_blob(wabi_word *val)
{
  wabi_word  l, x;
  char* c;

  l = WABI_WORD_VAL(*val);
  c = (char*)(val + 1);

  for(x = 0; x < l; x++) {
    putchar(*c);
    c++;
  }
}


void
wabi_pr_cont0(const wabi_vm vm, wabi_cont val)
{
  switch(WABI_TAG(val)) {
    case wabi_tag_cont_eval:
      printf("(EVAL)");
      break;
    case wabi_tag_cont_prompt:
      printf("(PROMPT ");
      wabi_pr(vm, (wabi_val) wabi_cont_prompt_tag((wabi_cont_prompt) val));
      printf(")");
      break;
    case wabi_tag_cont_apply:
      printf("(APPLY ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_apply) val)->args);
      printf(")");
      break;
    case wabi_tag_cont_call:
      printf("(CALL ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_call) val)->combiner);
      printf(")");
      break;
    case wabi_tag_cont_sel:
      printf("(SEL ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_sel) val)->left);
      printf(" ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_sel) val)->right);
      printf(")");
      break;
    case wabi_tag_cont_args:
      printf("(ARGS ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_args) val)->data);
      printf(" ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_args) val)->done);
      printf(")");
      break;
    case wabi_tag_cont_def:
      printf("(DEF ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_def) val)->pattern);
      printf(")");
      break;
    case wabi_tag_cont_prog:
      printf("(PROG ");
      wabi_pr(vm, (wabi_val) ((wabi_cont_prog) val)->expressions);
      printf(")");
      break;
    default:
      if(WABI_IS(wabi_tag_forward, val)) {
        wabi_pr_cont0(vm, (wabi_cont) WABI_DEREF((wabi_val) val));
        break;
      }
      if(wabi_atom_is_nil(vm, (wabi_val) val)) {
        break;
      }
      printf("(NAC ");
      wabi_pr(vm, (wabi_val) val);
      printf(")");
      break;
    }
}


void
wabi_pr_cont_combiner(const wabi_vm vm, wabi_combiner_continuation val)
{
  wabi_cont cont;
  cont = (wabi_cont) WABI_WORD_VAL(val->cont);

  printf("~cont(");

  do {
    wabi_pr_cont0(vm, cont);
    cont = (wabi_cont) wabi_cont_pop(cont);
    if(cont) printf(" ");
  } while(cont);

  printf(")");
}


void
wabi_pr_cont(const wabi_vm vm, wabi_cont val) {
  do {
    wabi_pr_cont0(vm, val);
    val = (wabi_cont) WABI_WORD_VAL(val->next);
    if(val) printf(" ");
  } while(val != NULL);
}



static void
wabi_pr_vector(const wabi_vm vm, wabi_vector d, wabi_size lvl);


static inline void
wabi_pr_vector_digit(const wabi_vm vm, wabi_vector_digit d, wabi_size lvl)
{
  wabi_size n, j;
  wabi_val t;

  n = wabi_vector_digit_node_size(d);
  t = wabi_vector_digit_table(d);

  if(lvl == 0) {
    for (j = 0; j < n; j++) {
      wabi_pr(vm, (wabi_val) *(t + j));
      if(j < n - 1) putchar(' ');
    }
    return;
  }
  for (j = 0; j < n; j++) {
    wabi_pr_vector(vm, (wabi_vector) *(t + j), lvl - 1);
    if(j < n - 1) putchar(' ');
  }
}


static inline void
wabi_pr_vector_deep(const wabi_vm vm, wabi_vector_deep d, wabi_size lvl)
{
  wabi_vector_digit l, r;
  wabi_vector m;

  l = wabi_vector_deep_left(d);
  m = wabi_vector_deep_middle(d);
  r = wabi_vector_deep_right(d);

  wabi_pr_vector_digit(vm, l, lvl);
  if(! wabi_vector_is_empty((wabi_val) m)) {
    putchar(' ');
    wabi_pr_vector(vm, m, lvl + 1);
  }
  putchar(' ');
  wabi_pr_vector_digit(vm, r, lvl);
}


static void
wabi_pr_vector(const wabi_vm vm, wabi_vector d, wabi_size lvl)
{
  switch(WABI_TAG(d)) {
  case wabi_tag_vector_digit:
    wabi_pr_vector_digit(vm, (wabi_vector_digit) d, lvl);
    return;
  case wabi_tag_vector_deep:
    wabi_pr_vector_deep(vm, (wabi_vector_deep) d, lvl);
    return;
  }
}

void
wabi_pr(const wabi_vm vm, wabi_val val) {
  switch(WABI_TAG(val)) {
  case wabi_tag_atom:
    if(wabi_atom_is_empty(vm, val)) {
      printf("()");
      break;
    }
    if(wabi_atom_is_ignore(vm, val)) {
      printf("_");
      break;
    }
    printf(":");
    wabi_pr_binary((wabi_binary) WABI_DEREF(val));
    break;
  case wabi_tag_fixnum:
    printf("%ld", WABI_CAST_INT64(val));
    break;
  case wabi_tag_symbol:
    wabi_pr_binary((wabi_binary) WABI_DEREF(val));
    break;
  case wabi_tag_pair:
    printf("(");
    wabi_pr_pair(vm, (wabi_pair) val);
    printf(")");
    break;
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    putchar('"');
    wabi_pr_binary((wabi_binary) val);
    putchar('"');
    break;
  case wabi_tag_bin_blob:
    printf("#blob\"");
    wabi_pr_binary_blob((wabi_word *) val);
    putchar('"');
    break;
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    putchar('{');
    wabi_pr_map(vm, (wabi_map) val);
    putchar('}');
    break;
  case wabi_tag_oper:
    wabi_pr_operative(vm, (wabi_combiner_derived) val);
    break;
  case wabi_tag_app:
    wabi_pr_applicative(vm, (wabi_combiner_derived) val);
    break;
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    printf("~B(%lu)", WABI_WORD_VAL(((wabi_combiner_builtin) val)->bid));
    break;
  case wabi_tag_cont_eval:
  case wabi_tag_cont_apply:
  case wabi_tag_cont_call:
  case wabi_tag_cont_sel:
  case wabi_tag_cont_args:
  case wabi_tag_cont_def:
  case wabi_tag_cont_prog:
  case wabi_tag_cont_prompt:
    printf("<");
    wabi_pr_cont(vm, (wabi_cont) val);
    printf(">");
    break;

  case wabi_tag_env:
    wabi_pr_env(vm, (wabi_env) val);
    break;
  case wabi_tag_forward:
    printf("F#");
    wabi_pr(vm, (wabi_val) WABI_WORD_VAL(*val));
    break;
  case wabi_tag_ct:
    wabi_pr_cont_combiner(vm, (wabi_combiner_continuation) val);
    break;
  case wabi_tag_place:
    printf("P#");
    wabi_pr(vm, (wabi_val) wabi_place_val((wabi_place) val));
    break;
  case wabi_tag_vector_digit:
  case wabi_tag_vector_deep:
    printf("[");
    wabi_pr_vector(vm, (wabi_vector) val, 0L);
    printf("]");
    break;
  default:
    printf("unknown %lx", WABI_TAG(val));
  }
}


void
  wabi_prn(const wabi_vm vm, wabi_val val)
{
  wabi_pr(vm, val);
  printf("\n");
}
