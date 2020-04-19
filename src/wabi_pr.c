#define wabi_pr_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_number.h"
#include "wabi_symbol.h"
#include "wabi_pair.h"
#include "wabi_combiner.h"
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


void wabi_pr_pair(wabi_pair val) {
  wabi_val car, cdr;

  do {
    car = wabi_car(val);
    cdr = wabi_cdr(val);

    if(WABI_IS(wabi_tag_pair, cdr)) {
      wabi_pr(car);
      putchar(' ');
      val = (wabi_pair) cdr;
      continue;
    }

    if(*cdr == wabi_val_nil) {
      wabi_pr(car);
      return;
    }

    wabi_pr(car);
    printf(" . ");
    wabi_pr(cdr);
    return;
  } while(1);
}


void
wabi_pr_map_entry(wabi_map_entry entry)
{
  wabi_pr((wabi_val) entry->key);
  putchar(' ');
  wabi_pr((wabi_val) WABI_WORD_VAL(entry->value));
}


void
wabi_pr_map(wabi_map map);


void
wabi_pr_map_array(wabi_map_array map)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word size = WABI_MAP_ARRAY_SIZE(map);
  for(int j = 0; j < size; j++) {
    wabi_map_entry row = (wabi_map_entry)(table + j);
    wabi_pr_map_entry(row);
    if(size - j - 1) {
      putchar(' ');
    }
  }
}


void
wabi_pr_map_hash(wabi_map_hash map)
{
  wabi_word bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  wabi_word size = WABI_MAP_BITMAP_COUNT(bitmap);
  for(int j = 0; j < size; j++) {
    wabi_pr_map(table + j);
    putchar(' ');
  }
}


void
wabi_pr_map(wabi_map map)
{
  switch(WABI_TAG(map)) {
  case wabi_tag_map_array:
    wabi_pr_map_array((wabi_map_array) map);
    return;
  case wabi_tag_map_hash:
    wabi_pr_map_hash((wabi_map_hash) map);
    return;
  case wabi_tag_map_entry:
    wabi_pr_map_entry((wabi_map_entry) map);
    return;
  default:
    // printf("fox %lx\n", *((wabi_word*) map));
    return;
  }
}


void
wabi_pr_env(wabi_env env)
{
  wabi_size j;

  printf("#env{");
  do {
    for(j = 0; j < env->numE; j++) {
      wabi_pr((wabi_val) *((wabi_word*) env->data + j * WABI_ENV_PAIR_SIZE));
      printf(" ");
    }
    env= (wabi_env) WABI_WORD_VAL(env->prev);
    if(env) printf(" -> ");
  } while(env);

  printf("}\n");
}


void
wabi_pr_applicative(wabi_combiner_derived val)
{
  wabi_val body;

  printf("(fn ");
  wabi_pr((wabi_val) val->parameters);
  printf(" ");
  body = (wabi_val) val->body;
  while(WABI_IS(wabi_tag_pair, body)) {
    wabi_pr(wabi_car((wabi_pair) body));
    body = wabi_cdr((wabi_pair) body);
    if(*body != wabi_val_nil) {
      printf(" ");
    }
  }
  printf(")");
}


void
wabi_pr_operative(wabi_combiner_derived val)
{
  wabi_val body;

  printf("(fx ");
  wabi_pr((wabi_val) val->caller_env_name);
  printf(" ");
  wabi_pr((wabi_val) val->parameters);
  printf(" ");
  body = (wabi_val) val->body;
  while(WABI_IS(wabi_tag_pair, body)) {
    wabi_pr(wabi_car((wabi_pair) body));
    body = wabi_cdr((wabi_pair) body);
    if(*body != wabi_val_nil) {
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
wabi_pr_cont0(wabi_cont val)
{
  switch(WABI_TAG(val)) {
    case wabi_tag_cont_eval:
      printf("(EVAL)");
      break;
    case wabi_tag_cont_prompt:
      printf("(PROMPT ");
      wabi_pr((wabi_val) ((wabi_cont_prompt) val)->tag);
      printf(")");
      break;
    case wabi_tag_cont_apply:
      printf("(APPLY ");
      wabi_pr((wabi_val) ((wabi_cont_apply) val)->args);
      printf(")");
      break;
    case wabi_tag_cont_call:
      printf("(CALL ");
      wabi_pr((wabi_val) ((wabi_cont_call) val)->combiner);
      printf(")");
      break;
    case wabi_tag_cont_sel:
      printf("(SEL ");
      wabi_pr((wabi_val) ((wabi_cont_sel) val)->left);
      printf(" ");
      wabi_pr((wabi_val) ((wabi_cont_sel) val)->right);
      printf(")");
      break;
    case wabi_tag_cont_args:
      printf("(ARGS ");
      wabi_pr((wabi_val) ((wabi_cont_args) val)->data);
      printf(" ");
      wabi_pr((wabi_val) ((wabi_cont_args) val)->done);
      printf(")");
      break;
    case wabi_tag_cont_def:
      printf("(DEF ");
      wabi_pr((wabi_val) ((wabi_cont_def) val)->pattern);
      printf(")");
      break;
    case wabi_tag_cont_prog:
      printf("(PROG ");
      wabi_pr((wabi_val) ((wabi_cont_prog) val)->expressions);
      printf(")");
      break;
    default:
      printf("BOH");
      break;
    }
}


void
wabi_pr_cont_combiner(wabi_combiner_continuation val)
{
  wabi_val tag;
  wabi_cont cont;
  tag = (wabi_val) WABI_WORD_VAL(val->tag);
  cont = (wabi_cont) val->cont;

  printf("~cont[");
  wabi_pr(tag);
  printf("](");

  do {
    if(WABI_IS(wabi_tag_cont_prompt, cont) && wabi_eq(tag, (wabi_val) ((wabi_cont_prompt) cont)->tag)) {
      break;
    }
    wabi_pr_cont0(cont);
    cont = (wabi_cont) WABI_WORD_VAL(cont->next);
  } while(1);

  printf(")");
}


void
wabi_pr_cont(wabi_cont val) {
  do {
    wabi_pr_cont0(val);
    val = (wabi_cont) WABI_WORD_VAL(val->next);
    if(val) printf(" ");
  } while(val != NULL);
}

void
wabi_pr(wabi_val val) {
  switch(WABI_TAG(val)) {
  case wabi_tag_constant:
    if(*val == wabi_val_nil) {
      printf("()");
    }
    if(*val == wabi_val_false) {
      printf("false");
    }
    if(*val == wabi_val_true) {
      printf("true");
    }
    if(*val == wabi_val_ignore) {
      printf("_");
    }
    /* switch(*val) { */
    /* case wabi_val_nil: */
    /*   printf("()"); */
    /*   break; */
    /* case wabi_val_false: */
    /*   break; */
    /* case wabi_val_true: */
    /*   printf("true"); */
    /*   break; */
    /* case wabi_val_ignore: */
    /*   printf("_"); */
    /*   break; */
    /* } */
    break;
  case wabi_tag_fixnum:
    printf("%ld", WABI_CAST_INT64(val));
    break;
  case wabi_tag_symbol:
    wabi_pr_binary((wabi_binary) WABI_DEREF(val));
    break;
  case wabi_tag_pair:
    printf("(");
    wabi_pr_pair((wabi_pair) val);
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
    wabi_pr_map((wabi_map) val);
    putchar('}');
    break;
  case wabi_tag_oper:
    wabi_pr_operative((wabi_combiner_derived) val);
    break;
  case wabi_tag_app:
    wabi_pr_applicative((wabi_combiner_derived) val);
    break;
  case wabi_tag_bt_app:
  case wabi_tag_bt_oper:
    printf("~B");
    wabi_pr((wabi_val) ((wabi_combiner_builtin) val)->c_name);
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
    wabi_pr_cont((wabi_cont) val);
    printf(">");
    break;

  case wabi_tag_env:
    wabi_pr_env((wabi_env) val);
    break;
  case wabi_tag_forward:
    printf("F#");
    wabi_pr((wabi_val) WABI_WORD_VAL(*val));
    break;
  case wabi_tag_ct_app:
  case wabi_tag_ct_oper:
    wabi_pr_cont_combiner((wabi_combiner_continuation) val);
    break;
  default:
    printf("unknown %lx", *val);
  }
}


void
wabi_prn(wabi_val val)
{
  wabi_pr(val);
  printf("\n");
}
