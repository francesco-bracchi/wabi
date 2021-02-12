#define wabi_builtin_c

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "wabi_value.h"
#include "wabi_list.h"
#include "wabi_env.h"
#include "wabi_combiner.h"
#include "wabi_cont.h"
#include "wabi_number.h"
#include "wabi_map.h"
#include "wabi_binary.h"
#include "wabi_reader.h"
#include "wabi_cmp.h"
#include "wabi_vm.h"
#include "wabi_error.h"
#include "wabi_atom.h"
#include "wabi_builtin.h"
#include "wabi_delim.h"
#include "wabi_error.h"
#include "wabi_pr.h"
#include "wabi_map.h"
#include "wabi_hash.h"
#include "wabi_symbol.h"
#include "wabi_place.h"
#include "wabi_vector.h"
#include "wabi_vm.h"


void
wabi_defx(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_builtin_fun fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner oper;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  oper = wabi_operator_builtin_new(vm, bin, fun);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, (wabi_val) oper);
}


void
wabi_defn(const wabi_vm vm,
          const wabi_env env,
          char* name,
          const wabi_builtin_fun fun)
{
  wabi_binary bin;
  wabi_symbol sym;
  wabi_combiner app;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  app = wabi_application_builtin_new(vm, bin, fun);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, (wabi_val) app);
}


void
wabi_def(const wabi_vm vm,
         const wabi_env env,
         char* name,
         wabi_val val)
{
  wabi_binary bin;
  wabi_symbol sym;

  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, name);
  if(vm->ert) return;
  sym = wabi_symbol_new(vm, (wabi_val) bin);
  if(vm->ert) return;
  wabi_env_set(vm, env, sym, val);
}


// todo: move in wabi_builtin.h
void
wabi_builtin_predicate(const wabi_vm vm,
                       const wabi_builtin_test_fn test_fn)
{
  wabi_val ctrl, val;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    val = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(! test_fn(val)) {
      vm->ctrl = vm->fls;
      vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
      return;
    }
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = vm->trh;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

void
wabi_builtin_def(const wabi_vm vm)
{
  wabi_val ctrl, lft, rgt, def, es;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env = (wabi_env) ((wabi_cont_call) cont)->env;
  cont = wabi_cont_pop(cont);

  // (def)
  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->cont = (wabi_val) cont;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  lft = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (def lft)
  if (wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_def(vm, env, lft, cont);
    if (vm->ert) return;
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val)cont;
    vm->env = (wabi_val)env;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  rgt = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (def lft rgt . ctrl)
  if(!wabi_atom_is_empty(vm, ctrl)) {
    def = (wabi_val) ((wabi_cont_call) vm->cont)->combiner;
    es = (wabi_val) wabi_cons(vm, def, ctrl);
    if(vm->ert) return;
    es = (wabi_val) wabi_cons(vm, es, vm->emp);
    if(vm->ert) return;
    cont = wabi_cont_push_prog(vm, env, es, cont);
    if(vm->ert) return;
  }
  cont = wabi_cont_push_def(vm, env, lft, cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;
  vm->ctrl = rgt;
  vm->cont = (wabi_val) cont;
  vm->env = (wabi_val) env;
}


void wabi_builtin_if (const wabi_vm vm)
{
  wabi_val ctrl, tst, lft, rgt, slf;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  env = (wabi_env) ((wabi_cont_call) cont)->env;
  cont = wabi_cont_pop(cont);

  // (if)
  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) cont;
    return;
  }

  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  tst = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  // (if tst)
  if (wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_eval(vm, cont);
    if (vm->ert) return;
    vm->ctrl = tst;
    vm->cont = (wabi_val)cont;
    vm->env = (wabi_val)env;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  // (if tst lft . ctrl)
  lft = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  slf = (wabi_val) ((wabi_cont_call) vm->cont)->combiner;
  rgt = (wabi_val) wabi_cons(vm, slf, ctrl);
  if(vm->ert) return;
  cont = wabi_cont_push_sel(vm, env, lft, rgt, cont);
  if(vm->ert) return;
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = tst;
  vm->cont = (wabi_val) cont;
  vm->env = (wabi_val) env;
}


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


static void
wabi_builtin_pr(const wabi_vm vm)
{
  wabi_val ctrl, v;

  ctrl = vm->ctrl;
  while(wabi_is_pair(ctrl)) {
    v = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    wabi_pr(vm, v);
    if(WABI_IS(wabi_tag_pair, ctrl)) printf(" ");
  }
  if (!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  printf("\n");
  vm->ctrl = vm->nil;
  vm->cont = (wabi_val)wabi_cont_pop((wabi_cont)vm->cont);
}

static void
wabi_builtin_eval(const wabi_vm vm)
{
  wabi_val ctrl, ex;
  wabi_env env;
  wabi_cont cont;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  env = (wabi_env) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_env((wabi_val) env)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  ex = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(vm->ert) return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = ex;
  vm->env = (wabi_val) env;
  vm->cont = (wabi_val) cont;
}


static void
wabi_builtin_do(const wabi_vm vm)
{
  wabi_val v, ctrl;
  wabi_cont cont;
  wabi_env env;

  ctrl = vm->ctrl;
  cont = (wabi_cont) vm->cont;
  cont = wabi_cont_pop(cont);

  if(wabi_atom_is_empty(vm, ctrl)) {
    vm->ctrl = vm->nil;
    vm->cont = (wabi_val) cont;
    return;
  }
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(wabi_is_pair(ctrl)) {
    env = (wabi_env) ((wabi_cont_call) vm->cont)->env;
    cont = wabi_cont_push_prog(vm, env, ctrl, cont);
    if(vm->ert) return;
  } else if (! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cont = wabi_cont_push_eval(vm, cont);
  if(vm->ert) return;

  vm->ctrl = v;
  vm->cont = (wabi_val) cont;
}

static void
wabi_builtin_clock(wabi_vm vm)
{
  wabi_val ctrl, res;

  ctrl = vm->ctrl;
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_fixnum_new(vm, clock() * 1000000 / CLOCKS_PER_SEC);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);;
}


static void
wabi_builtin_not(const wabi_vm vm)
{
  wabi_val v, ctrl;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(vm->ert) return;
  vm->ctrl = wabi_is_falsey(vm, v) ? vm->trh : vm->fls;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_builtin_hash(const wabi_vm vm)
{
  wabi_val ctrl, v, r;
  wabi_word hash;

  ctrl = vm->ctrl;
  if(! wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  v = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  hash = wabi_hash(v);
  if(! hash) {
    vm->ert = wabi_error_other;
    return;
  }
  r = wabi_fixnum_new(vm, hash);
  if(vm->ert) return;
  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}



static void
wabi_builtin_collect(const wabi_vm vm)
{
  /* wabi_size _free, total; */
  /* double perc; */
  /* time_t t, t0; */

  if(!wabi_atom_is_empty(vm, vm->ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  /* _free = wabi_store_free_words(&vm->stor); */
  /* total = wabi_store_size(&vm->stor); */
  /* perc = 100.0 * _free / total; */
  /* printf("BEFORE: %2.2f%% of free space\n", perc); */
  /* t0 = clock(); */
  wabi_vm_collect(vm);
  /* t = clock(); */
  /* _free = wabi_store_free_words(&vm->stor); */
  /* total = wabi_store_size(&vm->stor); */
  /* perc = 100.0 * _free / total; */
  /* printf("AFTER:  %2.2f%% of free space in %f ms \n", perc, 1000.0 * (t - t0) / CLOCKS_PER_SEC); */
  if(vm->ert) return;

  vm->ctrl = vm->nil;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


void
wabi_builtin_language0(wabi_vm vm)
{
  wabi_val ctrl, r;

  ctrl = vm->ctrl;
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  r = (wabi_val) wabi_builtin_stdenv(vm);
  if(vm->ert) return;

  vm->ctrl = r;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_builtin_load(const wabi_vm vm)
{
  wabi_val ctrl, exs;
  wabi_binary fn;
  char *cfn;
  FILE *f;
  long length;
  char *buffer;
  wabi_cont cont;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  fn = (wabi_binary) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_binary((wabi_val) fn)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  cfn = wabi_binary_to_cstring(vm, fn);
  if(vm->ert) return;

  f = fopen(cfn, "rb");
  printf("loading %s\n", cfn);
  if(! f)  {
    printf("error reading %s\n", cfn);
    vm->ert = wabi_error_file_not_found;
    return;
  }
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);
  exs = (wabi_val) wabi_reader_read_all(vm, buffer);
  free(buffer);
  if(vm->ert) return;
  cont = (wabi_cont) vm->cont;
  if(wabi_is_pair((wabi_val) exs)) {
    cont = wabi_cont_pop(cont);
    cont = wabi_cont_push_prog(vm, (wabi_env) vm->env, wabi_cdr((wabi_pair) exs), cont);
    if(vm->ert) return;
    cont = wabi_cont_push_eval(vm, cont);
    if(vm->ert) return;
    vm->ctrl = wabi_car((wabi_pair) exs);
    vm->cont = (wabi_val) cont;
    return;
  }

  vm->ctrl = exs;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}

wabi_env
wabi_builtin_stdenv(const wabi_vm vm)
{
  wabi_env env;

  env = wabi_env_new(vm);

  if(vm->ert) return NULL;

  wabi_defx(vm, env, "def", (wabi_builtin_fun) WABI_BT_DEF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "if", (wabi_builtin_fun) WABI_BT_IF);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "do", wabi_builtin_do);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "pr", wabi_builtin_pr);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "eval", wabi_builtin_eval);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "clock", wabi_builtin_clock);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "not", wabi_builtin_not);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "hash", wabi_builtin_hash);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "l0", wabi_builtin_language0);
  if(vm->ert) return NULL;

  wabi_defn(vm, env, "collect", wabi_builtin_collect);
  if(vm->ert) return NULL;

  wabi_defx(vm, env, "load", &wabi_builtin_load);
  if(vm->ert) return NULL;

  wabi_combiner_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_list_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_number_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_binary_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_cmp_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_env_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_delim_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_map_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_symbol_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_place_builtins(vm, env);
  if(vm->ert) return NULL;

  wabi_vector_builtins(vm, env);
  if(vm->ert) return NULL;

  return env;
}
