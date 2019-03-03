#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include "wabi_types.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_pair.h"

int main(int argc, char** argv)
{
  wabi_vm_t *vm = wabi_vm_new();
  if(! vm)  return 1;

  wabi_vm_init(vm, 1000);

  if(vm->errno) {
    printf("error errno %i", vm->errno);
    return 1;
  }

  printf("size: %lu\n", wabi_mem_used(vm));
  wabi_word_t *wnil = wabi_const_new(vm, WABI_VALUE_NIL);
  /* wabi_word_t *wtrue = wabi_const_new(vm, WABI_VALUE_TRUE); */
  /* wabi_word_t *wfalse = wabi_const_new(vm, WABI_VALUE_FALSE); */

  printf("size: %lu\n", wabi_mem_used(vm));
  wabi_const_new(vm, WABI_VALUE_TRUE);

  printf("size: %lu\n", wabi_mem_used(vm));
  wabi_const_new(vm, WABI_VALUE_FALSE);

  printf("size: %lu\n", wabi_mem_used(vm));
  wabi_word_t *wnum = wabi_small_new(vm, 123);

  wabi_pair_t* pair = (wabi_pair_t*) wabi_pair_cons(vm, wnum, wnil);
  wabi_word_t* w = (wabi_word_t*) pair;
  printf("size: %lu\n", wabi_mem_used(vm));

  wabi_mem_collect(vm, w);
  printf("size: %lu\n", wabi_mem_used(vm));

  free(vm);
  return 0;
}
