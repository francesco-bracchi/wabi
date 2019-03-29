#define wabi_pr_c

#include <stdio.h>

#include "wabi_object.h"
#include "wabi_pr.h"
#include "wabi_pair.h"
#include "wabi_binary.h"

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

void
wabi_pr(wabi_obj obj) {
  if(wabi_obj_is_nil(obj)) {
    printf("nil");
  } else if (wabi_obj_is_pair(obj)) {
    wabi_obj car = wabi_car_raw(obj);
    wabi_obj cdr = wabi_cdr_raw(obj);
    printf("(");
    wabi_pr(car);
    printf(" . ");
    wabi_pr(cdr);
    printf(")");
  } else if (wabi_obj_is_smallint(obj)) {
    printf("%li", *obj & WABI_VALUE_MASK);
  } else if (wabi_obj_is_bin(obj)) {
    putchar('"');
    wabi_pr_binary(obj);
    putchar('"');
  }
}
