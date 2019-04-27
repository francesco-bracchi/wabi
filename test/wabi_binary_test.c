#define wabi_binary_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_mem.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_binary.h"


void
wabi_binary_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 10 * 1024 * 1024); // 2MB

  wabi_mem_free(vm);
  free(vm);
}
