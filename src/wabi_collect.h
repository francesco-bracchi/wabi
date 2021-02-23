#ifndef wabi_collect_h

#define wabi_collect_h

#include "wabi_vm.h"
#include "wabi_value.h"

wabi_word*
wabi_copy_val(const wabi_vm vm, const wabi_val src);


void
wabi_collect(const wabi_vm vm);

#endif
