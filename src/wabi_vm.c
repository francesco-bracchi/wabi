#define wabi_vm_c

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_store.h"

void
wabi_vm_init(wabi_vm vm, wabi_size_t size)
{
  vm->errno = 0;
  wabi_store_init(wabi_vm_store(vm), size);
}


void
wabi_vm_free(wabi_vm vm) {
  wabi_store_free(wabi_vm_store(vm));
}
