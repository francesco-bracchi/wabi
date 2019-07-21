#define wabi_pr_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"
#include "wabi_symbol.h"
#include "wabi_env.h"

void
wabi_pr_binary(wabi_val val);


void
wabi_pr_bin_leaf(wabi_binary_leaf_t *leaf)
{
  char *from, *to, *c;
  from = (char*) leaf->data_ptr;
  to = from + (leaf->length & WABI_VALUE_MASK);

  for(c = from; c < to; c++)
    putchar(*c);
}


void
wabi_pr_bin_node(wabi_binary_node_t *node)
{
  wabi_pr_binary((wabi_val) node->left);
  wabi_pr_binary((wabi_val) node->right);
}


void wabi_pr_binary(wabi_val val)
{
  if(wabi_val_is_bin_leaf(val)) {
    wabi_pr_bin_leaf((wabi_binary_leaf_t *) val);
  }
  else if(wabi_val_is_bin_node(val)) {
    wabi_pr_bin_node((wabi_binary_node_t *) val);
  }
}


void wabi_pr_pair(wabi_val val) {
  wabi_val car, cdr;

  do {
    car = wabi_car_raw(val);
    cdr = wabi_cdr_raw(val);

    if (wabi_val_is_pair(cdr)) {
      wabi_pr(car);
      putchar(' ');
      val = cdr;
      continue;
    }

    if(wabi_val_is_nil(cdr)) {
      wabi_pr(car);
      return;
    }

    wabi_pr(car);
    printf(" . ");
    wabi_pr(cdr);
    return;
  } while(wabi_val_is_pair(val));
}


void
wabi_pr_map_entry(wabi_map_entry entry)
{
  wabi_pr((wabi_val) entry->key);
  putchar(' ');
  wabi_pr((wabi_val) (entry->value & WABI_VALUE_MASK));
}


void
wabi_pr_map(wabi_map map);


void
wabi_pr_map_array(wabi_map_array map)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  for(int j = 0; j < size; j++) {
    wabi_map_entry row = (wabi_map_entry)(table + j);
    wabi_pr_map_entry(row);
    putchar(' ');
  }
}


void
wabi_pr_map_hash(wabi_map_hash map)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  for(int j = 0; j < size; j++) {
    wabi_pr_map(table + j);
    putchar(' ');
  }
}


void
wabi_pr_map(wabi_map map)
{
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ARRAY:
    wabi_pr_map_array((wabi_map_array) map);
    return;
  case WABI_TAG_MAP_HASH:
    wabi_pr_map_hash((wabi_map_hash) map);
    return;
  case WABI_TAG_MAP_ENTRY:
    wabi_pr_map_entry((wabi_map_entry) map);
    return;
  default:
    // printf("fox %lx\n", *((wabi_word_t*) map));
    return;
  }
}


void
wabi_pr_env(wabi_env env)
{
  printf("#env{");
  do {
    wabi_pr_map((wabi_map) env->data);
    env = (wabi_env) (env->prev & WABI_VALUE_MASK);
    if(env == NULL) break;
    printf(";");
    break;
  } while(1);
  printf("}");
}


void
wabi_pr(wabi_val val) {
  switch(wabi_val_type(val)) {
  case WABI_TAG_NIL:
     printf("nil");
     break;
  case WABI_TAG_BOOL:
    printf(*val == WABI_VALUE_TRUE ? "true" : "false");
    break;
  case WABI_TAG_IGNORE:
    printf("#ignore");
    break;
  case WABI_TAG_SMALLINT:
    printf("%li", *val & WABI_VALUE_MASK);
    break;
  case WABI_TYPE_PAIR:
    printf("(");
    wabi_pr_pair(val);
    printf(")");
    break;
  case WABI_TYPE_BIN:
    putchar('"');
    wabi_pr_binary(val);
    putchar('"');
    break;
  case WABI_TYPE_MAP:
    putchar('{');
    wabi_pr_map((wabi_map) val);
    putchar('}');
    break;
  case WABI_TYPE_SYMBOL:
    wabi_pr_binary((wabi_val) WABI_SYMBOL_BINARY((wabi_symbol) val));
    break;
  case WABI_TYPE_ENV:
    wabi_pr_env((wabi_env) val);
    break;
  default:
    printf("unknown %lx", *val);
  }
}


/*** TEST: print internal map layout ***/

static inline void
print_indent(int x)
{
  for(int j = 0; j < x; j++)
    putchar(' ');
}

static void
wabi_map_pr_tree(wabi_map map,
                 int indent,
                 wabi_word_t hash_offset);

static void
wabi_map_pr_tree_array(wabi_map map,
                       int indent,
                       wabi_word_t hash_offset)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE((wabi_map_array) map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map);

  print_indent(indent);
  printf("ARRAY [\n");
  for(wabi_word_t index = 0; index < size; index++) {
    print_indent(indent);
    printf("%lu:\n", index);
    wabi_map_pr_tree(table + index, indent + 2, hash_offset - 6);
    printf("\n");
  }
  print_indent(indent);
  printf("]");
}


static void
wabi_map_pr_tree_hash(wabi_map map,
                      int indent,
                      wabi_word_t hash_offset)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE((wabi_map_hash) map);
  print_indent(indent);
  printf("HASH %lu {\n", hash_offset);
  for(wabi_word_t index = 0; index < 64; index++) {
    if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
      wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
      wabi_map submap = table + offset;
      print_indent(indent);
      printf("%lu:\n", index);
      wabi_map_pr_tree(submap, indent + 2, hash_offset - 6);
      printf("\n");
      offset++;
    }
  }

  print_indent(indent);
  printf("}");
}

static void
wabi_map_pr_tree(wabi_map map,
                 int indent,
                 wabi_word_t hash_offset)
{
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ENTRY:
    print_indent(indent);
    wabi_pr((wabi_val) map);
    break;
  case WABI_TAG_MAP_ARRAY:
    wabi_map_pr_tree_array(map, indent, hash_offset);
    break;
  case WABI_TAG_MAP_HASH:
    wabi_map_pr_tree_hash(map, indent, hash_offset);
    break;
  default:
    printf("unknown: %lx", wabi_val_tag((wabi_val) map));
  }
}

void
wabi_map_tree(wabi_map map)
{
  wabi_map_pr_tree(map, 0, WABI_MAP_INITIAL_OFFSET);
}
