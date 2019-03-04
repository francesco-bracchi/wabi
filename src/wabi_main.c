#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include "wabi_mem.h"
#include "wabi_err.h"

int main(int argc, char** argv)
{
  wabi_mem_init(1 * 1000 * 1000);
  if(wabi_errno) {
    printf("failed to initialize memory %i, %s\n", wabi_errno, wabi_err_msg(wabi_errno));
    return 1;
  }

  // wabi_word_t *foo = wabi_mem_allocate(10);

  printf("Hello, world\n");
  return 0;
}
