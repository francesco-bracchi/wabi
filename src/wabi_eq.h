#ifndef wabi_eq_h

#define wabi_eq_h

#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_vm.h"

/* int */
/* wabi_eq_c(wabi_obj a, wabi_obj b); */

wabi_obj
wabi_eq(wabi_vm vm, wabi_obj a, wabi_obj b);

int
wabi_eq_raw(wabi_obj a, wabi_obj b);

#endif
