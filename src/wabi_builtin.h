#ifndef wabi_builtin_h

#define wabi_builtin_h

#include "wabi_combiner.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_list.h"
#include "wabi_binary.h"
#include "wabi_error.h"
#include "wabi_symbol.h"


typedef int (*wabi_builtin_test_fn)(wabi_val);


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
  if (!wabi_is_empty((ctrl))) {                    \
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


void
wabi_builtin_def(const wabi_vm vm);

void
wabi_builtin_if(const wabi_vm vm);

#define WABI_BT_DEF 0
#define WABI_BT_IF 1
#define WABI_BT_LT 2
#define WABI_BT_SUM 3
#define WABI_BT_DIF 4

#endif
