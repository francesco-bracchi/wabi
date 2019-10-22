#ifndef wabi_builtin_h

#define wabi_builtin_h

#include "wabi_combiner.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_error.h"


#define WABI_BUILTIN_WRAP1(name, fun)                           \
  wabi_error_type                                               \
  name(wabi_vm vm)                                              \
  {                                                             \
    wabi_val ctrl, x;                                           \
    ctrl = vm->control;                                         \
    if(WABI_IS(wabi_tag_pair, ctrl)) {                          \
      x = wabi_car((wabi_pair) ctrl);                           \
      ctrl = wabi_cdr((wabi_pair) ctrl);                        \
      if(*ctrl == wabi_val_nil) {                               \
        return fun(vm, x);                                      \
      }                                                         \
    }                                                           \
    return wabi_error_bindings;                                 \
  }                                                             \


#define WABI_BUILTIN_WRAP2(name, fun)                           \
  wabi_error_type                                               \
  name(wabi_vm vm)                                              \
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
          return fun(vm, x, y);                                 \
        }                                                       \
      }                                                         \
    }                                                           \
    return wabi_error_bindings;                                 \
  }                                                             \


#define WABI_BUILTIN_WRAP3(name, fun)                           \
  wabi_error_type                                               \
  name(wabi_vm vm)                                              \
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
            return fun(vm, x, y, z);                            \
          }                                                     \
        }                                                       \
      }                                                         \
    }                                                           \
    return wabi_error_bindings;                                 \
  }                                                             \



#define SYM(vm, str)                                                    \
  wabi_symbol_new(vm,                                                   \
                  (wabi_val) wabi_binary_leaf_new_from_cstring(vm, str))


#define BTOPER(vm, str, fun)                                            \
  wabi_operator_builtin_new(vm,                                         \
                            (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, str), \
                            fun)                                        \


#define BTAPP(vm, str, fun)                                             \
  wabi_application_builtin_new(vm,                                      \
                               (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, str), \
                               fun)                                     \


#define WABI_DEFX(vm, env, name, btname, fun)                           \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) BTOPER(vm, btname, fun)                       \
               )


#define WABI_DEFN(vm, env, name, btname, fun)                           \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) BTAPP(vm, btname, fun)                        \
               )


#define WABI_DEF(vm, env, name, val)                                    \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               SYM(vm, name),                                           \
               (wabi_val) val                                           \
               )


wabi_env
wabi_builtin_stdenv(wabi_vm vm);


/*** this function doesn't belong to this header ***/
wabi_error_type
wabi_builtin_load(wabi_vm vm, wabi_env env, char* data);

#endif
