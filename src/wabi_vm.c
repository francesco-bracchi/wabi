
#define wabi_vm_c

#include <stdlib.h>
#include "wabi_vm.h"
#include "wabi_mem.h"

wabi_vm_t* wabi_vm_new()
{
  wabi_vm_t* res = (wabi_vm_t*) malloc(sizeof(wabi_vm_t));
  return res;
}

int wabi_vm_init(wabi_vm_t* vm, wabi_size_t size) {
  // wabi_mem_init(vm, size);
  return 0;
}
