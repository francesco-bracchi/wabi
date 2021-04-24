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
wabi_builtin_call(const wabi_vm vm,
                  const wabi_word func);

#define WABI_BT_EVAL 0
#define WABI_BT_CONS 1
#define WABI_BT_CAR 2
#define WABI_BT_CDR 3
#define WABI_BT_DEF 4
#define WABI_BT_SET 5
#define WABI_BT_IF 6
#define WABI_BT_DO 7
#define WABI_BT_PAIR_Q 8
#define WABI_BT_NUM_Q 9
#define WABI_BT_SYM_Q 10
#define WABI_BT_SYM_BIN 11
#define WABI_BT_ATOM_Q 12
#define WABI_BT_BIN_Q 13
#define WABI_BT_PLC_Q 14
#define WABI_BT_MAP_Q 15
#define WABI_BT_VEC_Q 16
#define WABI_BT_ENV_Q 17
#define WABI_BT_CONT_Q 18
#define WABI_BT_FX_Q 19
#define WABI_BT_FN_Q 20
#define WABI_BT_SUM 21
#define WABI_BT_MUL 22
#define WABI_BT_DIF 23
#define WABI_BT_DIV 24
#define WABI_BT_EQ 25
#define WABI_BT_NEQ 26
#define WABI_BT_GT 27
#define WABI_BT_LT 28
#define WABI_BT_GTE 29
#define WABI_BT_LTE 30
#define WABI_BT_PROMPT 31
#define WABI_BT_CONTROL 32
#define WABI_BT_MAP_NEW 33
#define WABI_BT_ASSOC 34
#define WABI_BT_DISSOC 35
#define WABI_BT_MAP_LEN 36
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
#define WABI_BT_VEC_SET 51
#define WABI_BT_FX 52
#define WABI_BT_FN 53
#define WABI_BT_WRAP 54
#define WABI_BT_UNWRAP 55
#define WABI_BT_LIST_Q 56
#define WABI_BT_NOT 57
#define WABI_BT_CLOCK 58
#define WABI_BT_PR 59
#define WABI_BT_LANGUAGE0 60
#define WABI_BT_LOAD 61
#define WABI_BT_LEN 62
#define WABI_BT_ENV_EXT 63
#define WABI_BT_HASH 64
#define WABI_BT_BIN_LEN 65
#define WABI_BT_BIN_CAT 66
#define WABI_BT_BIN_SUB 67
#define WABI_BT_COLLECT 68

#define WABI_BT_SIZE WABI_BT_COLLECT
#endif
