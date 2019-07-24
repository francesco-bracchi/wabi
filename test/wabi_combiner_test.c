#define wabi_combiner_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_combiner.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_store.h"


void
wabi_combiner_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 10MB

  wabi_vm_free(vm);
  free(vm);
}
