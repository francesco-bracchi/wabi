#define wabi_binary_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_store.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_cmp.h"


void
test_wabi_binary_collect_leaf(wabi_vm vm)
{
  wabi_val b = wabi_binary_new_from_cstring(vm, "a");
  wabi_store store = wabi_vm_store(vm);

  store->root = (wabi_word_t *) b;
  wabi_vm_collect(vm);

  wabi_val b0 = wabi_binary_new_from_cstring(vm, "a");
  ASSERT(wabi_cmp_raw(b0, store->root) == 0);
}

void
wabi_binary_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  wabi_vm_init(vm, 10 * 1024 * 1024); // 10MB

  test_wabi_binary_collect_leaf(vm);

  wabi_vm_free(vm);
  free(vm);
}
