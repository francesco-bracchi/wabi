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

wabi_env
wabi_builtin_stdenv(const wabi_vm vm);

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
wabi_builtin_call(const wabi_vm vm,
                  const wabi_word func);

#define WABI_BT_CONS 1
#define WABI_BT_CAR 2
#define WABI_BT_CDR 3
#define WABI_BT_DEF 4
#define WABI_BT_IF 5
#define WABI_BT_DO 6
#define WABI_BT_EVAL 7

#define WABI_BT_PAIR_Q 8
#define WABI_BT_NUM_Q 9
#define WABI_BT_SYM_Q 10
#define WABI_BT_ATOM_Q 11
#define WABI_BT_BIN_Q 12
#define WABI_BT_PLC_Q 13
#define WABI_BT_MAP_Q 14
#define WABI_BT_VEC_Q 15
#define WABI_BT_ENV_Q 16
#define WABI_BT_CONT_Q 17
#define WABI_BT_FX_Q 18
#define WABI_BT_FN_Q 19

#define WABI_BT_SUM 20
#define WABI_BT_MUL 21
#define WABI_BT_DIF 22
#define WABI_BT_DIV 23

#define WABI_BT_EQ 24
#define WABI_BT_NEQ 25
#define WABI_BT_GT 26
#define WABI_BT_LT 27
#define WABI_BT_GTE 28
#define WABI_BT_LTE 29

#define WABI_BT_PROMPT 30
#define WABI_BT_CONTROL 31

#define WABI_BT_MAP_NEW 32
#define WABI_BT_ASSOC 33
#define WABI_BT_DISSOC 34
#define WABI_BT_MAP_LEN 35
// #define WABI_BT_MAP_GET 36

#define WABI_BT_SYM 37
#define WABI_BT_ATOM 38


#define WABI_BT_PLC 39
#define WABI_BT_PLC_VAL 40
#define WABI_BT_PLC_CAS 41

#define WABI_BT_VEC 42
#define WABI_BT_VEC_PUSH_LEFT 43
#define WABI_BT_VEC_PUSH_RIGHT 44
#define WABI_BT_VEC_LEFT 45
#define WABI_BT_VEC_RIGHT 46
#define WABI_BT_VEC_POP_LEFT 47
#define WABI_BT_VEC_POP_RIGHT 48

#define WABI_BT_VEC_LEN 49
#define WABI_BT_VEC_CONCAT 50
// #define WABI_BT_VEC_REF 51
#define WABI_BT_VEC_SET 52


#define WABI_BT_FX 53
#define WABI_BT_FN 54
#define WABI_BT_WRAP 55
#define WABI_BT_UNWRAP 56
#define WABI_BT_LIST_Q 57

#define WABI_BT_NOT 58
#define WABI_BT_CLOCK 59
#define WABI_BT_PR 60
#define WABI_BT_L0 61
#define WABI_BT_LOAD 62
#define WABI_BT_LEN 63
#define WABI_BT_ENV_EXT 64
#define WABI_BT_HASH 65
#define WABI_BT_BIN_LEN 66
#define WABI_BT_BIN_CAT 67
#define WABI_BT_BIN_SUB 68
#define WABI_BT_COLLECT 69

#endif
