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

void
wabi_builtin_load_cstring(const wabi_vm vm, const wabi_env env, char* str)
{
  wabi_pair exs;
  wabi_cont cont;

  exs = (wabi_pair) wabi_reader_read_all(vm, str);
  if(vm->ert) return;
  if(! wabi_is_pair((wabi_val) exs)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cont = (wabi_cont) vm->cont;
  if(cont) cont = wabi_cont_pop(cont);
  cont = wabi_cont_push_prog(vm, env, wabi_cdr(exs), cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = wabi_car(exs);
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}

int
main(int argc,
     char* argv[])
{
  wabi_system_t sys;
  wabi_vm vm;
  wabi_env e0;
  char *buffer;
  long length;

  sys.config.store_size = 2500000;
  sys.config.fuel = 30000000;
  sys.config.num_threads = get_nprocs() + 1;

  wabi_system_init(&sys);
  vm = wabi_system_new_vm(&sys);
  if(! vm) {
    fprintf(stderr, "Not enough memory!\n");
    return 1;
  }
  if(argc < 2) {
    fprintf(stderr, "usage: wabi <filename>\n");
    return 2;
  }
  FILE * f = fopen(argv[1], "rb");
  if(!f) {
    fprintf(stderr, "Error opening file: %s\n", argv[1]);
    return 3;
  }

  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);
  e0 = wabi_builtin_stdenv(vm);
  wabi_builtin_load_cstring(vm, e0, buffer);
  free(buffer);
  wabi_system_run(&sys, vm);
  wabi_system_wait(&sys);
  return (int) vm->ert;
}
