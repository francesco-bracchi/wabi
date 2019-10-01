#ifndef wabi_builtin_h

#define wabi_builtin_h

#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"

#define WABI_BUILTIN_CONSTANT(name, val)                        \
  void                                                          \
  name(wabi_vm vm, wabi_env env)                                \
  {                                                             \
    wabi_val res;                                               \
    res = wabi_vm_alloc(vm, 1);                                 \
    if(res) {                                                   \
      *res = val;                                               \
      vm->control = res;                                        \
      return;                                                   \
    }                                                           \
  }                                                             \


#define WABI_BUILTIN_WRAP1_PLUS(name, fun)                      \
  void                                                          \
  name(wabi_vm vm, wabi_env env)                                \
  {                                                             \
    wabi_val ctrl, x, ac;                                       \
    ctrl = vm->control;                                         \
    if(WABI_IS(wabi_tag_pair, ctrl)) {                          \
      ac = wabi_car((wabi_pair) ctrl);                          \
      ctrl = wabi_cdr((wabi_pair) ctrl);                        \
      while(WABI_IS(wabi_tag_pair, ctrl)) {                     \
        x = wabi_car((wabi_pair) ctrl);                         \
        ctrl = wabi_cdr((wabi_pair) ctrl);                      \
        ac = fun(vm, env, ac, x);                               \
      }                                                         \
      vm->control = ac;                                         \
      return;                                                   \
    }                                                           \
    vm->errno = wabi_error_bindings;                            \
  }                                                             \


#define WABI_BUILTIN_WRAP1(name, fun)                           \
  void                                                          \
  name(wabi_vm vm, wabi_env env)                                \
  {                                                             \
    wabi_val ctrl, x;                                           \
    ctrl = vm->control;                                         \
    if(WABI_IS(wabi_tag_pair, ctrl)) {                          \
      x = wabi_car((wabi_pair) ctrl);                           \
      ctrl = wabi_cdr((wabi_pair) ctrl);                        \
      if(*ctrl == wabi_val_nil) {                               \
        vm->control = fun(vm, env, x);                          \
        return;                                                 \
      }                                                         \
    }                                                           \
    vm->errno = wabi_error_bindings;                            \
  }                                                             \


#define WABI_BUILTIN_WRAP2(name, fun)                           \
  void                                                          \
  name(wabi_vm vm, wabi_env env)                                \
  {                                                             \
    wabi_val ctrl, x, y;                                        \
    ctrl = vm->control;                                         \
    if(WABI_IS(wabi_tag_pair, ctrl)) {                          \
      x = wabi_car((wabi_pair) ctrl);                           \
      ctrl = wabi_cdr((wabi_pair) ctrl);                        \
      if(WABI_IS(wabi_tag_pair, ctrl)) {                        \
        y = wabi_car((wabi_pair) ctrl);                         \
        ctrl = wabi_cdr((wabi_pair) ctrl);                      \
        if(*ctrl == wabi_val_nil) {                             \
          vm->control = fun(vm, env, x, y);                     \
          return;                                               \
        }                                                       \
      }                                                         \
    }                                                           \
    vm->errno = wabi_error_bindings;                            \
  }                                                             \


#define WABI_BUILTIN_WRAP3(name, fun)                           \
  void                                                          \
  name(wabi_vm vm, wabi_env env)                                \
  {                                                             \
    wabi_val ctrl, x, y, z;                                     \
    ctrl = vm->control;                                         \
    if(WABI_IS(wabi_tag_pair, ctrl)) {                          \
      x = wabi_car((wabi_pair) ctrl);                           \
      ctrl = wabi_cdr((wabi_pair) ctrl);                        \
      if(WABI_IS(wabi_tag_pair, ctrl)) {                        \
        y = wabi_car((wabi_pair) ctrl);                         \
        ctrl = wabi_cdr((wabi_pair) ctrl);                      \
        if(WABI_IS(wabi_tag_pair, ctrl)) {                      \
          z = wabi_car((wabi_pair) ctrl);                       \
          ctrl = wabi_cdr((wabi_pair) ctrl);                    \
          if(*ctrl == wabi_val_nil) {                           \
            vm->control = fun(vm, env, x, y, z);                \
            return;                                             \
          }                                                     \
        }                                                       \
      }                                                         \
    }                                                           \
    vm->errno = wabi_error_bindings;                            \
  }                                                             \


wabi_env
wabi_builtin_stdenv(wabi_vm vm);

/*** this function doesn't belong to this header ***/
void
wabi_builtin_load(wabi_vm vm, wabi_env env, char* data);

#endif
