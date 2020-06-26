#ifndef wabi_builtin_h

#define wabi_builtin_h

#include "wabi_combiner.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_pair.h"
#include "wabi_binary.h"
#include "wabi_error.h"
#include "wabi_symbol.h"


typedef int (*wabi_builtin_test_fn)(wabi_val);


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
               (wabi_val) SYM(vm, name),                                \
               (wabi_val) BTOPER(vm, btname, fun)                       \
               )


#define WABI_DEFN(vm, env, name, btname, fun)                           \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               (wabi_val) SYM(vm, name),                                \
               (wabi_val) BTAPP(vm, btname, fun)                        \
               )


#define WABI_DEF(vm, env, name, val)                                    \
  wabi_env_set(vm,                                                      \
               env,                                                     \
               (wabi_val) SYM(vm, name),                                \
               (wabi_val) val                                           \
               )


#define WABI_BIND_START(vm)                     \
  wabi_val ctrl;                                \
  ctrl = (vm)->ctrl;

#define WABI_BIND(vm, t, v)                                             \
  if (!wabi_is_pair(ctrl)) {                                            \
    (vm)->ert = wabi_error_bindings;                                    \
    return;                                                             \
  }                                                                     \
  (v) = ((t))wabi_car((wabi_pair)ctrl);                                 \
  ctrl = wabi_cdr((wabi_pair)ctrl);                                     \
  if(!wabi_is_##t(vm)) {                                                \
    (vm)->ert = wabi_error_type_mismatch;                               \
    return;                                                             \
  }

#define WABI_BIND_END(vm)                          \
  if (!wabi_is_nil((ctrl))) {                      \
    (vm)->ert = wabi_error_bindings;               \
    return;                                        \
  }

wabi_env
wabi_builtin_stdenv(const wabi_vm vm);


void
wabi_builtin_load_cstring(const wabi_vm vm, const wabi_env env, char* data);


void
wabi_builtin_predicate(const wabi_vm vm, const wabi_builtin_test_fn fn);


void
wabi_def(const wabi_vm vm,
         const wabi_env env,
         char* name,
         wabi_val val);


void
wabi_defn(const wabi_vm vm,
          const wabi_env env,
         char* name,
          const wabi_builtin_fun fun);


void
wabi_defx(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_builtin_fun fun);

#endif
