#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "wabi_err.h"
#include "wabi_object.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_mem.h"
#include "wabi_pr.h"
#include "wabi_binary.h"

int main(int argc, char** argv)
{
  int errno;

  wabi_mem_init(1000, &errno);
  if(errno) {
    printf("failed to initialize memory %i, %s\n", errno, wabi_err_msg(errno));
    return 1;
  }

  wabi_obj x;
  wabi_obj nil;
  wabi_obj n0;
  wabi_obj n1;
  wabi_obj p0;
  wabi_obj p1;
  wabi_obj p2;
  wabi_obj b0;
  wabi_obj b1;
  wabi_obj b2;
  wabi_obj b3;
  //   wabi_obj b4;
  wabi_obj fr;
  wabi_obj ln;
  wabi_obj l;
  wabi_obj root;

  wabi_smallint(10, &x, &errno);
  wabi_nil(&nil, &errno);
  wabi_smallint(100, &n0, &errno);
  wabi_cons(n0, nil, &p0, &errno);
  wabi_smallint(200, &n1, &errno);
  wabi_cons(n1, p0,  &p1, &errno);
  wabi_cons(n1, p1,  &p2, &errno);
  wabi_binary_new_from_cstring("abcde", &b0, &errno);
  wabi_binary_new_from_cstring("0123456789", &b1, &errno);
  wabi_binary_concat(b0, b1, &b2, &errno);

  wabi_smallint(2, &fr, &errno);
  wabi_smallint(5, &ln, &errno);
  wabi_binary_sub(b2, fr, ln, &b3, &errno);

  wabi_binary_length(b3, &l, &errno);
  wabi_cons(b3, p1, &root, &errno);
  wabi_pr(root, &errno);
  printf("\n");
  wabi_mem_root = root;

  printf("used before collection %li\n", wabi_mem_used());
  wabi_mem_collect();
  printf("used after collection %li\n", wabi_mem_used());
  wabi_pr(wabi_mem_root, &errno);
  printf("\n");


}
