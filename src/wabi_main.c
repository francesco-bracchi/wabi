#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "wabi_err.h"
#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_pr.h"
#include "wabi_binary.h"
#include "wabi_hash.h"
#include "wabi_hamt.h"
#include "wabi_mem.h"


int main(int argc, char** argv)
{

  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 10000000);

  if(vm->errno) {
    printf("failed to initialize memory %i, %s\n", vm->errno, wabi_err_msg(vm->errno));
    return 1;
  }

  wabi_obj x = wabi_smallint(vm, 10);
  wabi_obj nil = wabi_nil(vm);
  wabi_obj n0 = wabi_smallint(vm, 100);
  wabi_obj n1 = wabi_smallint(vm, 200);
  wabi_obj p0 = wabi_cons(vm, n0, nil);
  wabi_obj p1 = wabi_cons(vm, n1, p0);
  wabi_obj p2 = wabi_cons(vm, n1, p1);
  wabi_obj fr = wabi_smallint(vm, 2);
  wabi_obj ln = wabi_smallint(vm, 5);
  wabi_obj b0 = wabi_binary_new_from_cstring(vm, "abcde");
  wabi_obj b1 = wabi_binary_new_from_cstring(vm, "0123456789");
  wabi_obj b2 = wabi_binary_concat(vm, b0, b1);
  wabi_obj b3 = wabi_binary_sub(vm, b2, fr, ln);
  wabi_obj l = wabi_binary_length(vm, b3);
  wabi_obj root = wabi_cons(vm, b3, p1);
  wabi_obj hash = 0;
  wabi_obj m0 = wabi_hamt_empty(vm);
  wabi_obj m1 = wabi_hamt_set(vm, m0, b0, n0);
  wabi_obj m2 = wabi_hamt_set(vm, m1, b3, n1);
  // wabi_obj g0 = wabi_hamt_get(vm, m1, b0);

  wabi_pr(m1);
  printf("\n");

  wabi_pr(m2);
  printf("\n");

  wabi_obj xx = wabi_hamt_get(vm, m2, b3);
  wabi_pr(xx);
  printf("\n");

  for(int j = 0; j < 40; j+=2) {
    printf("%i ", j);
    wabi_obj k = wabi_smallint(vm, j);
    wabi_obj v = wabi_binary_new_from_cstring(vm, "dest");
    m0 = wabi_hamt_set(vm, m0, k, v);

    wabi_pr(m0);
    printf("\n");
  }


  vm->mem_root = root;

  wabi_pr(vm->mem_root);
  printf("\n");


  hash = wabi_hash(vm, vm->mem_root);
  printf("HASH: ");
  wabi_pr(hash);
  printf("\n");

  printf("used before collection %li\n", wabi_mem_used(vm));
  wabi_mem_collect(vm);
  printf("used after collection %li\n", wabi_mem_used(vm));
  wabi_pr(vm->mem_root);
  printf("\n");


  hash = wabi_hash(vm, vm->mem_root);
  printf("HASH: ");
  wabi_pr(hash);
  printf("\n");


}
