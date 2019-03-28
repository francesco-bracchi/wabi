#ifndef wabi_atomic_h

#define wabi_atomic_h

#include <stdint.h>
#include "wabi_object.h"
#include "wabi_err.h"
#include "wabi_vm.h"

#define WABI_SMALLINT_SIZE 1
#define WABI_NIL_SIZE 1
#define WABI_BOOLEAN_SIZE 1

#define WABI_VALUE_NIL           0x0000000000000000
#define WABI_VALUE_FALSE         0x1000000000000000
#define WABI_VALUE_TRUE          0x1000000000000001

wabi_obj
wabi_smallint(wabi_vm vm, int64_t val);

wabi_obj
wabi_nil(wabi_vm vm);

wabi_obj
wabi_boolean(wabi_vm vm, int val);
#endif
