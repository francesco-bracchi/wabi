#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include "wabi_mem.h"
#include "wabi_types.h"
#include "wabi_err.h"
#include "wabi_pr.h"

int main(int argc, char** argv)
{
  wabi_mem_init(1 * 1000 * 1000 * 1000);
  if(wabi_errno) {
    printf("failed to initialize memory %i, %s\n", wabi_errno, wabi_err_msg(wabi_errno));
    return 1;
  }

  wabi_word_t *x = wabi_smallint_new(1);
  wabi_word_t *nil = wabi_nil_new();
  wabi_word_t *n0 = wabi_smallint_new(100);
  wabi_word_t *p0 = wabi_cons(n0, nil);
  wabi_word_t *n1 = wabi_smallint_new(200);
  wabi_word_t *p1 = wabi_cons(n1, p0);
  wabi_word_t *p2 = wabi_cons(n1, p1);

  wabi_mem_root = p2;

  wabi_pr(wabi_mem_root); printf("\n");

  printf("used %li\n", wabi_mem_used());
  wabi_mem_collect();

  printf("used %li\n", wabi_mem_used());
  wabi_pr(wabi_mem_root); printf("\n");
  return 0;
}
