#ifndef wabi_atom_h

#define wabi_atom_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_map.h"
#include "wabi_collect.h"

#define WABI_ATOM_SIZE 1

typedef wabi_word* wabi_atom;

wabi_atom
wabi_atom_new(const wabi_vm vm, const wabi_val bin_ref);


static inline wabi_val
wabi_atom_to_binary(const wabi_atom sym)
{
  return (wabi_val) WABI_WORD_VAL(*sym);
}


static inline void
wabi_atom_copy_val(const wabi_vm vm, const wabi_atom sym)
{
  wabi_copy_val_size(vm, (wabi_val) sym, WABI_ATOM_SIZE);
}


void
wabi_atom_collect_val(const wabi_vm vm, const wabi_val sym);


void
wabi_atom_builtins(const wabi_vm vm, const wabi_env env);


static inline int
wabi_is_atom(const wabi_val v) {
  return WABI_IS(wabi_tag_atom, v);
}


static inline void
wabi_atom_hash(const wabi_hash_state state, const wabi_atom val)
{
  wabi_hash_step(state, "A", 1);
  wabi_hash_val(state, wabi_atom_to_binary(val));
}

static inline int
wabi_atom_is_empty(const wabi_vm vm, const wabi_val v)
{
  return wabi_eq(v, vm->emp);
}

static inline int
wabi_atom_is_nil(const wabi_vm vm, const wabi_val v)
{
  return wabi_eq(v, vm->nil);
}

static inline int
wabi_atom_is_ignore(const wabi_vm vm, const wabi_val v)
{
  return wabi_eq(v, vm->ign);
}

static inline int
wabi_atom_is_true(const wabi_vm vm, const wabi_val v)
{
  return wabi_eq(v, vm->trh);
}

static inline int
wabi_atom_is_false(const wabi_vm vm, const wabi_val v)
{
  return wabi_eq(v, vm->fls);
}

static inline int
wabi_atom_is_boolean(const wabi_vm vm, const wabi_val v)
{
  return wabi_atom_is_false(vm, v) || wabi_atom_is_true(vm, v);
}

static inline int
wabi_is_falsey(const wabi_vm vm, const wabi_val v)
{
  return wabi_atom_is_false(vm, v) || wabi_atom_is_nil(vm, v);
}


static inline int
wabi_is_truthy(const wabi_vm vm, const wabi_val v)
{
  return !wabi_is_falsey(vm, v);
}

#endif