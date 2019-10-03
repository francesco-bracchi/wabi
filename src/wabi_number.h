#ifndef wabi_number_h

#define wabi_number_h

#include <stdint.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef wabi_val wabi_fixnum;

static const int wabi_fixnum_sign_offset = wabi_word_tag_offset - 1;

static const wabi_word wabi_fixnum_sign_mask = 1UL << wabi_fixnum_sign_offset;

#define FIXNUM_NEG(v) ((v) & wabi_fixnum_sign_mask)

#define WABI_CAST_INT64(v) ((int64_t) (FIXNUM_NEG(*v) ? *v | 0xF800000000000000 : WABI_WORD_VAL(*v)))

wabi_fixnum
wabi_fixnum_new(wabi_vm vm,
                int64_t val);


void
wabi_number_sum_builtin(wabi_vm vm, wabi_env env);


void
wabi_number_diff_builtin(wabi_vm vm, wabi_env env);


void
wabi_number_mul_builtin(wabi_vm vm, wabi_env env);


void
wabi_number_div_builtin(wabi_vm vm, wabi_env env);

#endif
