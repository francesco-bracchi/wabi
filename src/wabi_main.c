#define wabi_main

#include <stdio.h>
#include <stdlib.h>
#include <ev.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "wabi_system.h"
#include "wabi_env.h"
#include "wabi_vm.h"
#include "wabi_builtin.h"
#include "wabi_reader.h"
#include "wabi_cont.h"
#include "wabi_pr.h"

#define WABI_VM_FILL_TANK 100000

int
main(int argc,
     char* argv[])
{

  wabi_vm vm;
  wabi_env e0;
  char *buffer;
  long length;
  wabi_system_config_t config;

  config.store_size = 2500000;
  config.fuel = 3000000;
  config.num_threads = get_nprocs() + 1;

  wabi_system_init(&config);
  vm = wabi_system_new_vm();

  if(argc < 2) {
    fprintf(stderr, "usage: wabi <filename>\n");
    return 1;
  }

  FILE * f = fopen(argv[1], "rb");
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);

  e0 = wabi_builtin_stdenv(vm);
  wabi_builtin_load(vm, e0, buffer);
  wabi_system_run(vm);

  wabi_system_wait();
  return (int) vm->error;
}
