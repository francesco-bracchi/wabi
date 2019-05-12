#ifndef wabi_reader_h

#define wabi_reader_h

#include "wabi_vm.h"
#include <stdio.h>

wabi_val
wabi_read_raw(wabi_vm vm,
              FILE *fd);

#endif
