#define wabi_pr_c

#include <stdio.h>
#include "wabi_types.h"
#include "wabi_vm.h"
#include "wabi_pair.h"

void wabi_pr(wabi_vm_t *vm, wabi_word_t *obj) {
  if(wabi_is_nil(obj)) {
    printf("nil");
  } else if (wabi_is_pair(obj)) {
    wabi_word_t *car = wabi_pair_car(vm, obj);
    wabi_word_t *cdr = wabi_pair_cdr(vm, obj);
    printf("(");
    wabi_pr(vm, car);
    printf(" . ");
    wabi_pr(vm, cdr);
    printf(")");

  } else if (wabi_is_smallint(obj)) {
    printf("%li", wabi_value(obj));
  }
}
