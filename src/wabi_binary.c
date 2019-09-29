#define wabi_binary_c

#include "wabi_value.h"
#include "wabi_word.h"
#include "wabi_vm.h"
#include "wabi_binary.h"
#include <string.h>


wabi_binary_leaf
wabi_binary_leaf_new(wabi_vm vm, wabi_size size)
{
  wabi_size word_size, *blob;
  wabi_binary_leaf leaf;

  word_size = wabi_binary_word_size(size);

  blob = (wabi_word*) wabi_vm_alloc(vm, word_size + 1);
  WABI_SET_TAG(blob, wabi_tag_bin_blob);

  leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
  leaf->length = size;
  leaf->data_ptr = (wabi_word) blob;
  WABI_SET_TAG(leaf, wabi_tag_bin_leaf);

  return leaf;
}

wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(wabi_vm vm, char* cstring)
{
  wabi_size size;
  wabi_binary_leaf leaf;

  size = strlen(cstring);
  leaf = wabi_binary_leaf_new(vm, size);
  memcpy((char *) leaf->data_ptr, cstring, size);

  return leaf;
}

wabi_binary_node
wabi_binary_concat(wabi_vm vm, wabi_binary left, wabi_binary right)
{
  wabi_binary_node node;
  wabi_size length;

  length = wabi_binary_length(left) + wabi_binary_length(right);

  node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
  node->length = length;
  node->left = (wabi_word) left;
  node->right = (wabi_word) right;
  WABI_SET_TAG(node, wabi_tag_bin_node);

  return node;
}


wabi_binary
wabi_binary_sub_aux(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len);


wabi_binary
wabi_binary_sub_leaf(wabi_vm vm, wabi_binary_leaf leaf, wabi_size from, wabi_size len)
{
  wabi_binary_leaf new_leaf;

  new_leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
  new_leaf->length = len;
  new_leaf->data_ptr = leaf->data_ptr + from;
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

  return (wabi_binary) new_leaf;
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
    return wabi_binary_sub_aux(vm, left, from, len);
  } else if(pivot <= from) {
    return wabi_binary_sub_aux(vm, right, from - pivot, len);
  } else {
    new_node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
    new_node->length = len;
    new_node->left = (wabi_word) wabi_binary_sub_aux(vm, left, from, pivot - from);
    new_node->right = (wabi_word) wabi_binary_sub_aux(vm, right, 0, len + from - pivot);
    WABI_SET_TAG(new_node, wabi_tag_bin_node);

    return (wabi_binary) new_node;
  }
}


wabi_binary
wabi_binary_sub_aux(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len)
{
  return WABI_IS(wabi_tag_bin_leaf, bin)
    ? wabi_binary_sub_leaf(vm, (wabi_binary_leaf) bin, from, len)
    : wabi_binary_sub_node(vm, (wabi_binary_node) bin, from, len);
}


wabi_binary
wabi_binary_sub(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len)
{
  wabi_size bin_len;
  bin_len = wabi_binary_length(bin);
  if(from < 0 || len < 0 || from + len > bin_len)  return NULL;

  return (wabi_binary) wabi_binary_sub_aux(vm, bin, from, len);
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
