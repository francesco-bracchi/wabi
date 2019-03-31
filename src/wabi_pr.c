#define wabi_pr_c

#include <stdio.h>

#include "wabi_object.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_hamt.h"

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
wabi_pr_hamt(wabi_hamt_map obj)
{
  wabi_hamt_entry_t *entry = (wabi_hamt_entry_t *) obj->table;
  wabi_size_t cc = WABI_POPCNT(obj->bitmap);

  for(int j = 0; j < cc; j++) {
    if(wabi_obj_is_hamt((wabi_obj) entry)) {
      wabi_pr_hamt((wabi_hamt_map) entry);
    } else {
      wabi_pr((wabi_obj) (*entry).pair.key);
      putchar(' ');
      wabi_pr((wabi_obj) (*entry).pair.value);
    }
    entry++;
  }
}


void
wabi_pr(wabi_obj obj) {
  if(wabi_obj_is_nil(obj)) {
    printf("nil");
  } else if (wabi_obj_is_pair(obj)) {
    printf("(");
    wabi_pr_pair(obj);
    printf(")");
  } else if (wabi_obj_is_smallint(obj)) {
    printf("%li", *obj & WABI_VALUE_MASK);
  } else if (wabi_obj_is_bin(obj)) {
    putchar('"');
    wabi_pr_binary(obj);
    putchar('"');
  } else if (wabi_obj_is_hamt(obj)) {
    putchar('{');
    wabi_pr_hamt((wabi_hamt_map) obj);
    putchar('}');
  } else {
    printf("unknown");
  }
}
