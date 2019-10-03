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


wabi_binary_leaf
wabi_binary_leaf_new(wabi_vm vm, wabi_size size)
{
  wabi_size word_size, *blob;
  wabi_binary_leaf leaf;

  word_size = wabi_binary_word_size(size);
  if(wabi_vm_has_rooms(vm, word_size + 1 + WABI_BINARY_LEAF_SIZE)) {

    blob = (wabi_word*) wabi_vm_alloc(vm, word_size + 1);
    *blob = word_size + 1;
    WABI_SET_TAG(blob, wabi_tag_bin_blob);

    leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
    leaf->length = size;
    leaf->data_ptr = (wabi_word) blob;
    WABI_SET_TAG(leaf, wabi_tag_bin_leaf);

    return leaf;
  }
  return NULL;
}


wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(wabi_vm vm, char* cstring)
{
  wabi_size size;
  wabi_binary_leaf leaf;

  size = strlen(cstring);
  leaf = wabi_binary_leaf_new(vm, size);
  if(leaf)
    memcpy((char *) leaf->data_ptr, cstring, size);

  return leaf;
}


wabi_binary
wabi_binary_sub(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len);


wabi_binary
wabi_binary_sub_leaf(wabi_vm vm, wabi_binary_leaf leaf, wabi_size from, wabi_size len)
{
  wabi_binary_leaf new_leaf;

  if(wabi_vm_has_rooms(vm, WABI_BINARY_LEAF_SIZE)) {
    new_leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
    new_leaf->length = len;
    new_leaf->data_ptr = leaf->data_ptr + from;
    WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

    return (wabi_binary) new_leaf;
  }
  return NULL;
}


wabi_binary
wabi_binary_sub_node(wabi_vm vm, wabi_binary_node node, wabi_size from, wabi_size len)
{
  wabi_binary_node new_node;
  wabi_binary left, right;
  wabi_size pivot;

  left = (wabi_binary) node->left;
  right = (wabi_binary) node->right;
  pivot = wabi_binary_length(left);

  if(pivot > from + len) {
    return wabi_binary_sub(vm, left, from, len);
  } else if(pivot <= from) {
    return wabi_binary_sub(vm, right, from - pivot, len);
  } else {
    if(wabi_vm_has_rooms(vm, WABI_BINARY_NODE_SIZE)) {
      new_node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
      new_node->length = len;
      new_node->left = (wabi_word) wabi_binary_sub(vm, left, from, pivot - from);
      new_node->right = (wabi_word) wabi_binary_sub(vm, right, 0, len + from - pivot);
      WABI_SET_TAG(new_node, wabi_tag_bin_node);

      return (wabi_binary) new_node;
    }
  }
  return NULL;
}


wabi_binary
wabi_binary_sub(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len)
{
  return WABI_IS(wabi_tag_bin_leaf, bin)
    ? wabi_binary_sub_leaf(vm, (wabi_binary_leaf) bin, from, len)
    : wabi_binary_sub_node(vm, (wabi_binary_node) bin, from, len);
}


inline static wabi_val
wabi_binary_concat_bt(wabi_vm vm, wabi_env env, wabi_val l, wabi_val r)
{
  wabi_binary_node node;
  wabi_size length;
  if(wabi_binary_p(l) && wabi_binary_p(r)) {
    if(wabi_vm_has_rooms(vm, WABI_BINARY_NODE_SIZE)) {
      length = wabi_binary_length((wabi_binary) l) + wabi_binary_length((wabi_binary) r);
      node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
      node->length = length;
      node->left = (wabi_word) l;
      node->right = (wabi_word) r;
      WABI_SET_TAG(node, wabi_tag_bin_node);
      return (wabi_val) node;
    }
    vm->errno = wabi_error_nomem;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


inline static wabi_val
wabi_binary_sub_bt(wabi_vm vm, wabi_env env, wabi_val bin, wabi_val from, wabi_val len)
{
  wabi_word f, l, l0;
  wabi_val res;
  if(wabi_binary_p(bin) &&
     WABI_IS(wabi_tag_fixnum, from) &&
     WABI_IS(wabi_tag_fixnum, len)) {

    f = WABI_CAST_INT64(from);
    l = WABI_CAST_INT64(len);
    l0 = wabi_binary_length((wabi_binary) bin);

    if(f >= 0L && f < l0 && l >= 0L && l < l0 - f) {
      res = (wabi_val) wabi_binary_sub(vm, (wabi_binary) bin, f, l);
      if(res) return res;
      vm->errno = wabi_error_nomem;
      return vm->control;
    }
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}

inline static wabi_val
wabi_binary_length_bt(wabi_vm vm, wabi_env env, wabi_val bin)
{
  wabi_val res;
  if(wabi_binary_p(bin)) {
    if(wabi_vm_has_rooms(vm, 1)) {
      res = wabi_vm_alloc(vm, 1);
      *res = wabi_binary_length((wabi_binary) bin);
      WABI_SET_TAG(res, wabi_tag_fixnum);
      return res;
    }
    vm->errno = wabi_error_nomem;
    return vm->control;
  }
  vm->errno = wabi_error_type_mismatch;
  return vm->control;
}


void
wabi_binary_memcopy(char *dst, wabi_binary src) {
  wabi_word pivot;

  if(WABI_IS(wabi_tag_bin_leaf, src)) {
    memcpy(dst, (char *)((wabi_binary_leaf) src)->data_ptr, wabi_binary_length(src));
  } else {
    pivot = wabi_binary_length((wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst, (wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst + pivot, (wabi_binary) ((wabi_binary_node) src)->left);
  }
}

WABI_BUILTIN_WRAP1(wabi_binary_length_builtin, wabi_binary_length_bt);
WABI_BUILTIN_WRAP2(wabi_binary_concat_builtin, wabi_binary_concat_bt);
WABI_BUILTIN_WRAP3(wabi_binary_sub_builtin, wabi_binary_sub_bt);
