#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include "wabi_types.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_pair.h"
#include "wabi_pr.h"

int main(int argc, char** argv)
{
  wabi_vm_t *vm = wabi_vm_new();
  if(! vm)  return 1;

  wabi_vm_init(vm, 1000);

  if(vm->errno) {
    printf("error errno %i", vm->errno);
    return 1;
  }

  wabi_word_t *nil = wabi_const_new(vm, WABI_VALUE_NIL);

  wabi_const_new(vm, WABI_VALUE_TRUE);
  wabi_const_new(vm, WABI_VALUE_FALSE);
  wabi_small_new(vm, 01);
  wabi_word_t *n0 = wabi_small_new(vm, 123);
  wabi_word_t *n1 = wabi_small_new(vm, 124);
  wabi_word_t* pair = wabi_pair_cons(vm, n0, nil);
  wabi_word_t* pair1 = wabi_pair_cons(vm, n0, pair);
  vm->mem_root = pair1;

  wabi_pr(vm, vm->mem_root);
  printf("\n\n");

  printf("before collect used: %lu\n", wabi_mem_used(vm));
  wabi_mem_collect(vm);
  printf("after collect used: %lu\n", wabi_mem_used(vm));

  wabi_pr(vm, vm->mem_root);

  /* wabi_mem_collect(vm); */
  /* printf("after collect used: %lu\n", wabi_mem_used(vm)); */

  return 0;
}
