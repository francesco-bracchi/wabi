#define wabi_main

#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

#include "wabi_system.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_builtin.h"
#include "wabi_reader.h"
#include "wabi_cont.h"

int
main(int argc,
     char* argv[])
{

  wabi_vm_t vm;
  wabi_env e0;
  char *buffer;
  long length;

  wabi_vm_init(&vm, 25000);

  FILE * f = fopen("test/test.wabi", "rb");
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);

  e0 = wabi_builtin_stdenv(&vm);
  wabi_builtin_load(&vm, e0, buffer);
  wabi_vm_run(&vm);
  printf("error: %i\n", vm.error);
  /* wabi_prn(vm.control); */
  /* wabi_prn(vm.continuation); */

  wabi_vm_destroy(&vm);
  return (int) vm.error;
}
