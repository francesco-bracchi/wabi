#ifndef wabi_atomic_h

#define wabi_atomic_h

#include <stdint.h>
#include "wabi_object.h"
#include "wabi_err.h"

#define WABI_SMALLINT_SIZE 1
#define WABI_NIL_SIZE 1
#define WABI_BOOLEAN_SIZE 1

#define WABI_VALUE_NIL           0x0000000000000000
#define WABI_VALUE_FALSE         0x1000000000000000
#define WABI_VALUE_TRUE          0x1000000000000001

void
wabi_smallint(int64_t val, wabi_obj *res, wabi_error *err);

void
wabi_nil(wabi_obj *res, int *err);

void
wabi_boolean(int val, wabi_obj *res, wabi_error *err);
#endif
