#define wabi_pr_c

#include <stdio.h>
#include "wabi_types.h"
#include "wabi_pr.h"

void wabi_pr(wabi_word_t *obj) {
  if(wabi_is_nil(obj)) {
    printf("nil");
  } else if (wabi_is_pair(obj)) {
    wabi_word_t *car = wabi_car(obj);
    wabi_word_t *cdr = wabi_cdr(obj);
    printf("(");
    wabi_pr(car);
    printf(" . ");
    wabi_pr(cdr);
    printf(")");

  } else if (wabi_is_smallint(obj)) {
    printf("%li", wabi_type_value(obj));
  }
}
