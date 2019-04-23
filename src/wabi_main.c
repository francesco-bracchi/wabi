#define wabi_main_c

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "wabi_err.h"
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_pr.h"
#include "wabi_binary.h"
#include "wabi_hash.h"
#include "wabi_map.h"
#include "wabi_mem.h"
#include "wabi_symbol.h"
#include "time.h"

int main(int argc, char** argv)
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 30 * 1024 * 1024);

  if(vm->errno) {
    printf("failed to initialize memory %i, %s\n", vm->errno, wabi_err_msg(vm->errno));
    return 1;
  }

  wabi_val x = wabi_smallint(vm, 10);
  wabi_val nil = wabi_nil(vm);
  wabi_val n0 = wabi_smallint(vm, 100);
  wabi_val n1 = wabi_smallint(vm, 200);
  wabi_val p0 = wabi_cons(vm, n0, nil);
  wabi_val p1 = wabi_cons(vm, n1, p0);
  wabi_val p2 = wabi_cons(vm, n1, p1);
  wabi_val fr = wabi_smallint(vm, 2);
  wabi_val ln = wabi_smallint(vm, 5);
  wabi_val b0 = wabi_binary_new_from_cstring(vm, "abcde");
  wabi_val b1 = wabi_binary_new_from_cstring(vm, "0123456789");
  wabi_val b2 = wabi_binary_concat(vm, b0, b1);
  wabi_val b3 = wabi_binary_sub(vm, b2, fr, ln);
  wabi_val l = wabi_binary_length(vm, b3);
  wabi_val root = wabi_cons(vm, b3, p1);
  wabi_val hash = 0;
  wabi_val m0 = wabi_map_empty(vm);
  wabi_val lm0;

  clock_t start, end;
  double cpu_time_used;

  wabi_val k;
  wabi_val v;

  char str[80];

  wabi_word_t foo = 43UL;

  start = clock();
  for(wabi_word_t j = 0; j < foo; j++) {
    sprintf(str, "%lu", j);
    k = wabi_smallint(vm, j);
    v = wabi_binary_new_from_cstring(vm, str);
    m0 = wabi_map_assoc(vm, m0, k, v);
  }

  /* lm0 = wabi_map_length(vm, m0); */
  /* printf("LENGTH: "); */
  /* wabi_pr(lm0); */
  /* printf("\n"); */

  /* for(wabi_word_t j = 0; j < foo - 1; j++) { */
  /*   sprintf(str, "%lu", j); */
  /*   k = wabi_smallint(vm, j); */
  /*   m0 = wabi_map_dissoc(vm, m0, k); */
  /* } */

  /* lm0 = wabi_map_length(vm, m0); */
  /* printf("LENGTH: "); */
  /* wabi_pr(lm0); */
  /* printf("\n"); */
  /* // wabi_pr(m0); */

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("TIME USED %f\n", cpu_time_used);

  wabi_val s3 = wabi_intern(vm, b3);
  wabi_val s4 = wabi_symbol(vm, b0);
  wabi_val s5 = wabi_intern(vm, b1);

  m0 = wabi_map_assoc(vm, m0, s3, wabi_map_empty(vm));
  // printf("M0\n");
  // wabi_pr(m0);
  // printf("\n");
  // m0 = wabi_map_dissoc(vm, m0, wabi_smallint(vm, 2));

  vm->mem_root = m0;

  hash = wabi_hash(vm, vm->mem_root);
  printf("HASH: ");
  wabi_pr(hash);
  printf("\n");
  // wabi_pr(vm->mem_root);
  printf("\n");
  printf("symbol table:\n");
  wabi_pr(vm->symbol_table);
  printf("\nused before collection %p %li\n", vm->mem_root, wabi_mem_used(vm));

  start = clock();
  wabi_mem_collect(vm);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("used  after collection %p %li\n", vm->mem_root, wabi_mem_used(vm));
  printf("TIME USED %f\n", cpu_time_used);

  // wabi_pr(vm->mem_root);
  printf("\n");
  printf("symbol table:\n");
  wabi_pr(vm->symbol_table);
  printf("\n");

  hash = wabi_hash(vm, vm->mem_root);
  printf("HASH: ");
  wabi_pr(hash);
  printf("\n");
}
