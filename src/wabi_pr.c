#define wabi_pr_c

#include <stdio.h>

#include "wabi_object.h"
#include "wabi_pr.h"
#include "wabi_pair.h"

void
wabi_pr(wabi_obj obj, int* errno) {
  if(wabi_obj_is_nil(obj)) {
    printf("nil");
  } else if (wabi_obj_is_pair(obj)) {
    wabi_obj car;
    wabi_obj cdr;
    wabi_car(obj, &car, errno);
    wabi_cdr(obj, &cdr, errno);
    printf("(");
    wabi_pr(car, errno);
    printf(" . ");
    wabi_pr(cdr, errno);
    printf(")");

  } else if (wabi_obj_is_smallint(obj)) {
    printf("%li", *obj & WABI_VALUE_MASK);
  }
}
