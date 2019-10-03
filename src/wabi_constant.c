#define wabi_constant_c

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_constant.h"
#include "wabi_error.h"
#include "wabi_builtin.h"


static inline wabi_val
wabi_constant_nil_p(wabi_vm vm,
                    wabi_env env,
                    wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == wabi_val_nil ? wabi_val_true : wabi_val_false;
    return res;
  }
  vm->errno = wabi_error_nomem;
}


static inline wabi_val
wabi_constant_ignore_p(wabi_vm vm,
                       wabi_env env,
                       wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == wabi_val_ignore ? wabi_val_true : wabi_val_false;
    return res;
  }
  vm->errno = wabi_error_nomem;
}


static inline wabi_val
wabi_constant_boolean_p(wabi_vm vm,
                        wabi_env env,
                        wabi_val v)
{
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = *v == (wabi_val_true || *v == wabi_val_false) ? wabi_val_true : wabi_val_false;
    return res;
  }
  vm->errno = wabi_error_nomem;
}

WABI_BUILTIN_CONSTANT(wabi_constant_builtin_nil, wabi_val_nil)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_true, wabi_val_true)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_false, wabi_val_false)
WABI_BUILTIN_CONSTANT(wabi_constant_builtin_ignore, wabi_val_ignore)

WABI_BUILTIN_WRAP1(wabi_constant_builtin_nil_p, wabi_constant_nil_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_ignore_p, wabi_constant_ignore_p)
WABI_BUILTIN_WRAP1(wabi_constant_builtin_boolean_p, wabi_constant_boolean_p)
