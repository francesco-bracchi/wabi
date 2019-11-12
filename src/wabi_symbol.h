#ifndef wabi_symbol_h

#define wabi_symbol_h

#include "wabi_vm.h"
#include "wabi_value.h"

#define WABI_SYMBOL_SIZE 2

typedef struct wabi_symbol_struct {
  wabi_word binref;
  wabi_word uid;
} wabi_symbol_t;

typedef wabi_symbol_t* wabi_symbol;

wabi_symbol
wabi_symbol_new(wabi_vm vm, wabi_val bin_ref);


static inline wabi_val
wabi_symbol_to_binary(wabi_symbol sym)
{
  return (wabi_val) WABI_WORD_VAL(sym->binref);
}
#endif
