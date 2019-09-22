#ifndef wabi_reader_h

#define wabi_reader_h

#include "wabi_vm.h"

typedef struct wabi_reader_state_struct {
  wabi_vm vm;
  wabi_pair stack;
} wabi_reader_state_t

typedef wabi_reader_state_t* wabi_reader_state;

#endif
