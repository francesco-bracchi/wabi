#ifndef wabi_eq_h

#define wabi_eq_h

#include "wabi_object.h"
#include "wabi_err.h"

/* int */
/* wabi_eq_c(wabi_obj a, wabi_obj b); */

void
wabi_eq(wabi_obj a, wabi_obj b, wabi_obj *res, wabi_error *err);

#endif
