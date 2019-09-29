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

  if (!ev_default_loop (0)) {
    printf ("could not initialise libev, bad $LIBEV_FLAGS in environment?");
    exit(1);
  }

  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(pr-str \"Hello World.\")");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);
  return 0;
}
