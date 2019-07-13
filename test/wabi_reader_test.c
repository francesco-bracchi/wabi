#define wabi_reader_test_c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_assert.h"

#include "../src/wabi_value.h"
#include "../src/wabi_err.h"
#include "../src/wabi_vm.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_atomic.h"
#include "../src/wabi_map.h"
#include "../src/wabi_pr.h"
#include "../src/wabi_reader.h"


void
wabi_reader_test()
{
  wabi_vm vm = (wabi_vm) malloc(sizeof(wabi_vm_t));
  vm->errno = 0;
  wabi_vm_init(vm, 100 * 1024 * 1024); // 100MB

  FILE *fd = fopen("test/test.wabi", "r");

  if(! fd) {
    printf("file not found\n");
  }
  wabi_val expr = wabi_read_raw(vm, fd);
  if(vm->errno) {
    printf("error: %i\n", vm->errno);
  }
  ASSERT(expr != NULL);

  wabi_vm_free(vm);
}
