#define wabi_constant_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_builtin.h"

WABI_BUILTIN_CONSTANT(wabi_constant_builtin_nil, wabi_val_nil)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_true, wabi_val_true)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_false, wabi_val_false)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_ignore, wabi_val_ignore)

WABI_BUILTIN_WRAP1(wabi_constant_builtin_nil_p, WABI_CONST_NIL_P)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_ignore_p, WABI_CONST_IGNORE_P)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_boolean_p, WABI_CONST_BOOLEAN_P)
