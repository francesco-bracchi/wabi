#define wabi_pr_c

#include <stdio.h>

#include "wabi_value.h"
#include "wabi_atomic.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"


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
  for(int j = 0; j < size; j++)
    wabi_pr_map(table + size);
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
  }
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
  default:
    printf("unknown %lx", *val);
  }
}
