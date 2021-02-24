#ifndef wabi_number_h

#define wabi_number_h

#include <stdint.h>
#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef wabi_val wabi_fixnum;

static const int wabi_fixnum_sign_offset = wabi_word_tag_offset - 1;

static const wabi_word wabi_fixnum_sign_mask = 1UL << wabi_fixnum_sign_offset;

static const wabi_word wabi_fixnum_max = wabi_word_value_mask;

#define FIXNUM_NEG(v) ((v) & wabi_fixnum_sign_mask)

#define WABI_CAST_INT64(v) ((int64_t) (FIXNUM_NEG(*v) ? *v | 0xFF00000000000000 : WABI_WORD_VAL(*v)))

wabi_fixnum
wabi_fixnum_new(const wabi_vm vm,
                const int64_t val);

static inline int
wabi_is_fixnum(const wabi_val val)
{
  return WABI_IS(wabi_tag_fixnum, val);
}
void
wabi_number_builtin_sum(const wabi_vm vm);

void
wabi_number_builtin_dif(const wabi_vm vm);

#endif
