#ifndef wabi_reader_h

#define wabi_reader_h

#include "wabi_vm.h"
#include "wabi_value.h"

wabi_val
wabi_reader_read(const wabi_vm vm, char* c);

wabi_val
wabi_reader_read_val(const wabi_vm vm, char** c);

wabi_val
wabi_reader_read_all(const wabi_vm vm, char* c);

#endif
