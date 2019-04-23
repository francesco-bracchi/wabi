#define test_run_c

#include "wabi_cmp_test.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  int r;

  r = wabi_cmp_test();
  if(r == 0) {
    printf("OK\n");
    return 0;
  }
  return 1;
}
