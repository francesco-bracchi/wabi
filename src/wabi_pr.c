#define wabi_pr_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_number.h"
/* #include "wabi_symbol.h" */
/* #include "wabi_combiner.h" */
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

    if(wabi_tag_pair == WABI_TAG(cdr)) {
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
  } while(wabi_tag_pair == WABI_TAG(val));
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


/* void */
/* wabi_pr_env(wabi_env env) */
/* { */
/*   printf("#env{"); */
/*   do { */
/*     wabi_pr_map((wabi_map) env->data); */
/*     env = (wabi_env) (env->prev & WABI_VALUE_MASK); */
/*     if(env == NULL) break; */
/*     printf(";"); */
/*     break; */
/*   } while(1); */
/*   printf("}"); */
/* } */


/* void */
/* wabi_pr_applicative(wabi_combiner_derived val) */
/* { */
/*   printf("(fn "); */
/*   wabi_pr((wabi_val) val->arguments); */
/*   printf(" "); */
/*   wabi_pr((wabi_val) val->body); */
/*   printf(")"); */
/* } */


/* void */
/* wabi_pr_operative(wabi_combiner_derived val) */
/* { */
/*   printf("(fx "); */
/*   wabi_pr((wabi_val) val->caller_env_name); */
/*   printf(" "); */
/*   wabi_pr((wabi_val) val->arguments); */
/*   printf(" "); */
/*   wabi_pr((wabi_val) val->body); */
/*   printf(")"); */
/* } */

/* /\** todo: add a tag name to the builtin ds *\/ */
/* void */
/* wabi_pr_combiner(wabi_val val) { */
/*   switch(wabi_val_tag(val)) { */
/*   case WABI_TAG_OPERATIVE: */
/*     wabi_pr_operative((wabi_combiner_derived) val); */
/*     break; */
/*   case WABI_TAG_APPLICATIVE: */
/*     wabi_pr_applicative((wabi_combiner_derived) val); */
/*     break; */
/*   case WABI_TAG_BUILTIN_OP: */
/*     printf("builtin operative"); */
/*     break; */
/*   case WABI_TAG_BUILTIN_APP: */
/*     printf("builtin applicative"); */
/*     break; */
/*   } */
/* } */

void
wabi_pr(wabi_val val) {
  switch(WABI_TAG(val)) {
  case wabi_tag_constant:
    switch(*val) {
    case wabi_val_nil:
      printf("nil");
      break;
    case wabi_val_false:
      printf("false");
      break;
    case wabi_val_true:
      printf("true");
      break;
    case wabi_val_ignore:
      printf("_");
      break;
    }
    /* case WABI_TAG_SMALLINT: */
    /*   printf("%li", *val & WABI_VALUE_MASK); */
    /*   break; */
  case wabi_tag_fixnum:
    printf("%ld", WABI_CAST_INT64(val));
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
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    putchar('{');
    wabi_pr_map((wabi_map) val);
    putchar('}');
    break;
    /* case WABI_TYPE_SYMBOL: */
    /*   wabi_pr_binary((wabi_val) WABI_SYMBOL_BINARY((wabi_symbol) val)); */
    /*   break; */
    /* case WABI_TYPE_ENV: */
    /*   wabi_pr_env((wabi_env) val); */
    /*   break; */
    /* case WABI_TYPE_COMBINER: */
    /*   wabi_pr_combiner(val); */
    /*   break; */
  default:
    printf("unknown %lx", *val);
  }
}
