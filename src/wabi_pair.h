#ifndef wabi_pair_h

#define wabi_pair_h

#include "wabi_types.h"
#include "wabi_vm.h"

wabi_word_t *wabi_pair_cons(wabi_vm_t* vm, wabi_word_t *a, wabi_word_t* d);
wabi_word_t *wabi_small_new(wabi_vm_t* vm, long long a);
wabi_word_t *wabi_const_new(wabi_vm_t* vm, wabi_word_t c);
wabi_word_t *wabi_binary_new(wabi_vm_t* vm, wabi_word_t size);
wabi_word_t *wabi_binary_copy(wabi_vm_t* vm, void* ptr, wabi_word_t size);

#endif
