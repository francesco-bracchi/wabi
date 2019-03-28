#define wabi_vm_c

#include "wabi_vm.h"
#include "wabi_object.h"


int wabi_vm_init(wabi_vm_t* vm, wabi_size_t size)
{
  vm->errno = 0;
  wabi_mem_init(vm, size);
}
