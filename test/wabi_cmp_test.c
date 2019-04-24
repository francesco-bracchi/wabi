#define wabi_cmp_test_c

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_mem.h"
#include "wabi_vm.h"
#include "wabi_atomic.h"
#include "wabi_binary.h"
#include "wabi_cmp.h"

#include <string.h>
#include <stdlib.h>


#define ASSERT(e) if(!(e)){printf("assertion failed: a <= b"); return 1}

int
test_wabi_binary_compare_different_string_same_length(wabi_vm vm)
{
  wabi_val foo = wabi_binary_new_from_cstring(vm, "foo");
  wabi_val bar = wabi_binary_new_from_cstring(vm, "bar");

  return wabi_cmp_raw(foo, bar) < 0;
}


int
wabi_cmp_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_mem_init(vm, 10 * 1024 * 1024); // 10MB

  return test_wabi_binary_compare_different_string_same_length(vm);
}
