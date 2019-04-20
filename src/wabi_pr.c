#define wabi_pr_c

#include <stdio.h>

#include "wabi_object.h"
#include "wabi_atomic.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_map.h"


void
wabi_pr_binary(wabi_obj obj);


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
  wabi_pr_binary((wabi_obj) node->left);
  wabi_pr_binary((wabi_obj) node->right);
}


void wabi_pr_binary(wabi_obj obj)
{
  if(wabi_obj_is_bin_leaf(obj)) {
    wabi_pr_bin_leaf((wabi_binary_leaf_t *) obj);
  }
  else if(wabi_obj_is_bin_node(obj)) {
    wabi_pr_bin_node((wabi_binary_node_t *) obj);
  }
}


void wabi_pr_pair(wabi_obj obj) {
  wabi_obj car, cdr;

  do {
    car = wabi_car_raw(obj);
    cdr = wabi_cdr_raw(obj);

    if (wabi_obj_is_pair(cdr)) {
      wabi_pr(car);
      putchar(' ');
      obj = cdr;
      continue;
    }

    if(wabi_obj_is_nil(cdr)) {
      wabi_pr(car);
      return;
    }

    wabi_pr(car);
    printf(" . ");
    wabi_pr(cdr);
    return;
  } while(wabi_obj_is_pair(obj));
}


void
wabi_pr_map_entry(wabi_map_entry entry)
{
  wabi_pr((wabi_obj) entry->key);
  putchar(' ');
  wabi_pr((wabi_obj) (entry->value & WABI_VALUE_MASK));
}


void
wabi_pr_map_array(wabi_map_array map)
{
  wabi_map_table table = (wabi_map_table) WABI_MAP_ARRAY_SIZE(map);
  wabi_word_t size = WABI_MAP_ARRAY_TABLE(map);
  for(int j = 0; j < size; j++) {
    wabi_map_entry row = (wabi_map_entry)(table + j);
    wabi_pr_map_entry(row);
    putchar(' ');
  }
}


void
wabi_pr_map(wabi_map_table map);

void
wabi_pr_map_hash(wabi_map_hash map)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_map_table table = (wabi_map_table) WABI_MAP_HASH_TABLE(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  for(int j = 0; j < size; j++)
    wabi_pr_map(table + size);
}

void
wabi_pr_map(wabi_map_table map)
{
  switch(wabi_obj_tag((wabi_obj) map)) {
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
wabi_pr(wabi_obj obj) {
  switch(wabi_obj_tag(obj)) {
  case WABI_TAG_NIL:
     printf("nil");
     break;
  case WABI_TAG_BOOL:
    printf(*obj == WABI_VALUE_TRUE ? "true" : "false");
    break;
  case WABI_TAG_IGNORE:
    printf("#ignore");
    break;
  case WABI_TAG_SMALLINT:
    printf("%li", *obj & WABI_VALUE_MASK);
    break;
  case WABI_TAG_PAIR:
    printf("(");
    wabi_pr_pair(obj);
    printf(")");
    break;
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    putchar('"');
    wabi_pr_binary(obj);
    putchar('"');
    break;
  case WABI_TAG_MAP_HASH:
  case WABI_TAG_MAP_ARRAY:
    putchar('{');
    wabi_pr_map((wabi_map_table) obj);
    putchar('}');
    break;
  default:
    printf("unknown %lx", *obj);
  }
}
