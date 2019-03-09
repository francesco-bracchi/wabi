#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include "wabi_err.h"
#include "wabi_object.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_mem.h"
#include "wabi_pr.h"

int main(int argc, char** argv)
{
  int errno;

  wabi_mem_init(1000, &errno);
  if(errno) {
    printf("failed to initialize memory %i, %s\n", errno, wabi_err_msg(errno));
    return 1;
  }

  wabi_obj x;
  wabi_obj  nil;
  wabi_obj  n0;
  wabi_obj  n1;
  wabi_obj  p0;
  wabi_obj  p1;
  wabi_obj  p2;

  wabi_smallint(10, &x, &errno);
  wabi_nil(&nil, &errno);
  wabi_smallint(100, &n0, &errno);
  wabi_cons(n0, nil, &p0, &errno);
  wabi_smallint(200, &n1, &errno);
  wabi_cons(n1, p0,  &p1, &errno);
  wabi_cons(n1, p1,  &p2, &errno);
  wabi_mem_root = p1;

  wabi_pr(p1, &errno);
  printf("\n");
  printf("used %li\n", wabi_mem_used());
  wabi_mem_collect();
  printf("used %li\n", wabi_mem_used());

  /* // wabi_pr(wabi_mem_root); printf("\n"); */

}
