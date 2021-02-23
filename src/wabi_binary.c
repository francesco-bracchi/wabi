/***
 * Binaries
 *
 * Basic operations:
 * 1. Length in bytes
 * 2. concatenate
 * 3. subsequence
 *
 * Binaries are represented as "strings", altough they do no carry information
 * about the encoding.
 */

#define wabi_binary_c

#include <string.h>
#include "wabi_value.h"
#include "wabi_word.h"
#include "wabi_vm.h"
#include "wabi_env.h"
#include "wabi_binary.h"
#include "wabi_error.h"
#include "wabi_number.h"
#include "wabi_builtin.h"
#include "wabi_error.h"
#include "wabi_env.h"
#include "wabi_atom.h"


wabi_binary_leaf
wabi_binary_leaf_new(const wabi_vm vm,
                     const wabi_size size)
{
  wabi_size word_size, *blob;
  wabi_binary_leaf leaf;

  word_size = wabi_binary_word_size(size);
  blob = (wabi_word*) wabi_vm_alloc(vm, word_size + 1);
  if(vm->ert) return NULL;

  *blob = word_size + 1;
  WABI_SET_TAG(blob, wabi_tag_bin_blob);

  leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
  if(vm->ert) return NULL;

  leaf->length = size;
  leaf->data_ptr = (wabi_word) blob;
  WABI_SET_TAG(leaf, wabi_tag_bin_leaf);

  return leaf;
}


wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(const wabi_vm vm,
                                  const char* cstring)
{
  wabi_binary_leaf bin;
  unsigned long len;

  len = strlen(cstring);
  bin = wabi_binary_leaf_new(vm, len);
  if(vm->ert) return NULL;

  memcpy((char*) bin->data_ptr, cstring, len);
  return bin;
}


wabi_binary_node
wabi_binary_node_new(const wabi_vm vm,
                     const wabi_binary left,
                     const wabi_binary right)
{
  wabi_word len;
  wabi_binary_node bin;

  len = wabi_binary_length(left) + wabi_binary_length(right);
  bin = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
  if(vm->ert) return NULL;

  bin->length = len;
  bin->left = (wabi_word) left;
  bin->right = (wabi_word) right;
  WABI_SET_TAG(bin, wabi_tag_bin_node);

  return bin;
}


wabi_binary
wabi_binary_sub(const wabi_vm vm,
                const wabi_binary bin,
                const wabi_size from,
                const wabi_size len);


wabi_binary
wabi_binary_sub_leaf(const wabi_vm vm,
                     const wabi_binary_leaf leaf,
                     const wabi_size from,
                     const wabi_size len)
{
  wabi_binary_leaf new_leaf;

  new_leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
  if(vm->ert) return NULL;

  new_leaf->length = len;
  new_leaf->data_ptr = leaf->data_ptr + from;
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

  return (wabi_binary) new_leaf;
}


wabi_binary
wabi_binary_sub_node(const wabi_vm vm,
                     const wabi_binary_node node,
                     const wabi_size from,
                     const wabi_size len)
{
  wabi_binary_node new_node;
  wabi_binary left, right;
  wabi_size pivot;

  left = (wabi_binary) node->left;
  right = (wabi_binary) node->right;
  pivot = wabi_binary_length(left);

  if(pivot > from + len) {
    return wabi_binary_sub(vm, left, from, len);
  }
  if(pivot <= from) {
    return wabi_binary_sub(vm, right, from - pivot, len);
  }

  left = wabi_binary_sub(vm, left, from, pivot - from);
  if(vm->ert) return NULL;

  right = wabi_binary_sub(vm, right, 0, len + from - pivot);
  if(vm->ert) return NULL;

  new_node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
  if(vm->ert) return NULL;

  new_node->length = len;
  new_node->left = (wabi_word) left;
  new_node->right = (wabi_word) right;
  WABI_SET_TAG(new_node, wabi_tag_bin_node);

  return (wabi_binary) new_node;
}


wabi_binary
wabi_binary_sub(const wabi_vm vm,
                const wabi_binary bin,
                const wabi_size from,
                const wabi_size len)
{
  return WABI_IS(wabi_tag_bin_leaf, bin)
    ? wabi_binary_sub_leaf(vm, (wabi_binary_leaf) bin, from, len)
    : wabi_binary_sub_node(vm, (wabi_binary_node) bin, from, len);
}

/**
 * collecting
 */


/**
 * Hashing
 */

static void
wabi_binary_hash_generic(const wabi_hash_state state, const wabi_binary bin);

static inline void
wabi_binary_leaf_hash(const wabi_hash_state state, const wabi_binary_leaf leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, wabi_binary_length((wabi_binary) leaf));
}

static inline void
wabi_binary_node_hash(const wabi_hash_state state, const wabi_binary_node node)
{
  wabi_binary_hash_generic(state, (wabi_binary) node->left);
  wabi_binary_hash_generic(state, (wabi_binary) node->right);
}

static void
wabi_binary_hash_generic(const wabi_hash_state state, const wabi_binary bin)
{
  switch(WABI_TAG(bin)) {
  case wabi_tag_bin_leaf:
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    break;
  case wabi_tag_bin_node:
    wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
    break;
  }
}

void
wabi_binary_hash(const wabi_hash_state state,
                 const wabi_binary bin)
{
  wabi_hash_step(state, "B", 1);
  wabi_binary_hash_generic(state, bin);
}

/**
 * Builtins
 */

static void
wabi_binary_bin_concat(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_binary bin0, bin;

  ctrl = vm->ctrl;
  bin = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, "");
  if(vm->ert) return;

  while(wabi_is_pair(ctrl)) {
    bin0 = (wabi_binary) wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(!wabi_is_binary((wabi_val) bin0)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }
    if(wabi_binary_length(bin0)) {
      bin = (wabi_binary) wabi_binary_node_new(vm, bin, bin0);
    }
    if(vm->ert) return;
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  vm->ctrl = (wabi_val) bin;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_binary_bin_sub(const wabi_vm vm)
{
  wabi_binary bin;
  wabi_fixnum offset, len;
  wabi_val ctrl;
  wabi_word os, l, l0;

  ctrl = vm->ctrl;
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  bin = (wabi_binary) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_binary((wabi_val) bin)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  offset = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_fixnum((wabi_val) offset)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_is_pair(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }

  len = wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);
  if(! wabi_is_fixnum((wabi_val) len)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  if(!wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  os = WABI_CAST_INT64(offset);
  l = WABI_CAST_INT64(len);
  l0 = wabi_binary_length(bin);

  if(os < 0 || os >= l0 || l < 0 || l >= l0 - os) {
    vm->ert = wabi_error_out_of_range;
    return;
  }

  bin = wabi_binary_sub(vm, bin, os, l);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) bin;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_binary_bin_length(const wabi_vm vm)
{
  wabi_val ctrl;
  wabi_binary bin;
  wabi_size sum;
  wabi_fixnum res;

  ctrl = vm->ctrl;
  sum = 0L;

  while(wabi_is_pair(ctrl)) {
    bin = (wabi_binary) wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(!wabi_is_binary((wabi_val) bin)) {
      vm->ert = wabi_error_type_mismatch;
      return;
    }

    sum += wabi_binary_length(bin);
  }
  if(! wabi_atom_is_empty(vm, ctrl)) {
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  res = wabi_fixnum_new(vm, sum);
  if(vm->ert) return;

  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_pop((wabi_cont) vm->cont);
}


static void
wabi_binary_bin_p(const wabi_vm vm)
{
  wabi_binary bin;
  wabi_val ctrl;

  ctrl = vm->ctrl;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    bin = (wabi_binary) wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(! wabi_is_binary((wabi_val) bin)) {
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


static inline void
wabi_binary_memcopy(char *dst, wabi_binary src) {
  wabi_word pivot;

  if(WABI_IS(wabi_tag_bin_leaf, src)) {
    memcpy(dst, (char *)((wabi_binary_leaf) src)->data_ptr, wabi_binary_length(src));
  } else {
    // todo use a lop
    pivot = wabi_binary_length((wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst, (wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst + pivot, (wabi_binary) ((wabi_binary_node) src)->right);
  }
}

char*
wabi_binary_to_cstring(const wabi_vm vm,
                       const wabi_binary bin)
{
  char* blob;
  wabi_size len;

  len = wabi_binary_length(bin);
  blob = (char*) wabi_vm_alloc(vm, len + 2);
  if(vm->ert) return NULL;

  WABI_SET_TAG(blob, wabi_tag_bin_blob);

  wabi_binary_memcopy(blob + 1, bin);
  *(blob + len + 1) = '\0';
  return blob + 1;
}


void
wabi_binary_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defn(vm, env, "bin?", &wabi_binary_bin_p);
  if(vm->ert) return;
  wabi_defn(vm, env, "bin-len", &wabi_binary_bin_length);
  if(vm->ert) return;
  wabi_defn(vm, env, "bin-cat", &wabi_binary_bin_concat);
  if(vm->ert) return;
  wabi_defn(vm, env, "bin-sub", &wabi_binary_bin_sub);
}
