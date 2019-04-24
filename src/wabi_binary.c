#define wabi_binary_c

#define WABI_BINARY_LEAF_SIZE 3

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_err.h"
#include "wabi_mem.h"
#include "wabi_atomic.h"
#include "wabi_binary.h"

wabi_size_t
wabi_binary_word_size(wabi_size_t size)
{
  wabi_size_t word_size;
  word_size = size >> 2;
  if(size & 3) word_size++;
  return word_size;
}

wabi_val
wabi_blob_new(wabi_vm vm, wabi_size_t size)
{
  size ++;
  wabi_val res = wabi_mem_allocate(vm, size);
  if(vm->errno) return NULL;
  *res = size | WABI_TAG_BIN_BLOB;
  return res + 1;
}

wabi_val
wabi_binary_new(wabi_vm vm, wabi_size_t size)
{
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_mem_allocate(vm, WABI_BINARY_LEAF_SIZE);
  if(vm->errno) return NULL;

  wabi_size_t word_size = wabi_binary_word_size(size);
  wabi_word_t* blob = (wabi_word_t*) wabi_mem_allocate(vm, word_size + 1);
  if(vm->errno) return NULL;

  leaf->length = size | WABI_TAG_BIN_LEAF;
  leaf->data_ptr = (wabi_word_t) blob;

  return (wabi_val) leaf;
}

wabi_val
wabi_binary_new_from_cstring(wabi_vm vm, char* cstring)
{
  wabi_size_t size = strlen(cstring);
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_binary_new(vm, size);
  if(vm->errno) return NULL;
  memcpy((char *) leaf->data_ptr, cstring, size);
  return (wabi_val) leaf;
}


wabi_size_t
wabi_binary_length_raw(wabi_binary bin)
{
  return WABI_BINARY_LENGTH(bin);
}


wabi_val
wabi_binary_length(wabi_vm vm, wabi_val bin)
{
  switch(wabi_val_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    return wabi_smallint(vm, WABI_BINARY_LENGTH((wabi_binary) bin));
  default:
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
}

wabi_binary
wabi_binary_concat_raw(wabi_vm vm, wabi_binary left, wabi_binary right)
{
  wabi_binary_node node = (wabi_binary_node) wabi_mem_allocate(vm, WABI_BINARY_NODE_SIZE);
  if(vm->errno) return NULL;

  wabi_size_t length = WABI_BINARY_LENGTH(left) + WABI_BINARY_LENGTH(right);
  node->length = length | WABI_TAG_BIN_NODE;
  node->left = (wabi_word_t) left;
  node->right = (wabi_word_t) right;
  return (wabi_binary) node;
}


wabi_val
wabi_binary_concat(wabi_vm vm, wabi_val left, wabi_val right)
{
  if(wabi_val_is_forward(left)) {
    left = (wabi_val) (*left & WABI_VALUE_MASK);
  }
  if(wabi_val_is_forward(right)) {
    right = (wabi_val) (*right & WABI_VALUE_MASK);
  }
  if(wabi_val_is_bin(left) && wabi_val_is_bin(right)) {
    return (wabi_val) wabi_binary_concat_raw(vm, (wabi_binary) left, (wabi_binary) right);
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}

static wabi_binary
wabi_binary_sub_aux(wabi_vm vm, wabi_binary bin, wabi_size_t from, wabi_size_t len);


static wabi_binary
wabi_binary_sub_leaf(wabi_vm vm, wabi_binary_leaf leaf, wabi_size_t from, wabi_size_t len)
{
  wabi_binary_leaf new_leaf = (wabi_binary_leaf) wabi_mem_allocate(vm, WABI_BINARY_LEAF_SIZE);
  if(vm->errno) return NULL;

  new_leaf->length = len | WABI_TAG_BIN_LEAF;
  new_leaf->data_ptr = leaf->data_ptr + from;

  return (wabi_binary) new_leaf;
}

static wabi_binary
wabi_binary_sub_node(wabi_vm vm, wabi_binary_node node, wabi_size_t from, wabi_size_t len)
{
  wabi_binary left = (wabi_binary) node->left;
  wabi_binary right = (wabi_binary) node->right;
  wabi_size_t pivot = WABI_BINARY_LENGTH(left);

  if(pivot > from + len) {
    return wabi_binary_sub_aux(vm, left, from, len);
  } else if(pivot <= from) {
    return wabi_binary_sub_aux(vm, right, from - pivot, len);
  } else {
    wabi_binary_node new_node = (wabi_binary_node) wabi_mem_allocate(vm, WABI_BINARY_NODE_SIZE);
    if(vm->errno) return NULL;

    wabi_binary new_left = wabi_binary_sub_aux(vm, left, from, pivot - from);
    if(vm->errno) return NULL;

    wabi_binary new_right = wabi_binary_sub_aux(vm, right, 0, len + from - pivot);
    if(vm->errno) return NULL;

    new_node->length = len | WABI_TAG_BIN_NODE;
    new_node->left = (wabi_word_t) new_left;
    new_node->right = (wabi_word_t) new_right;

    return (wabi_binary) new_node;
  }
}

static wabi_binary
wabi_binary_sub_aux(wabi_vm vm, wabi_binary bin, wabi_size_t from, wabi_size_t len)
{
  return wabi_val_is_bin_leaf((wabi_val) bin)
    ? wabi_binary_sub_leaf(vm, (wabi_binary_leaf) bin, from, len)
    : wabi_binary_sub_node(vm, (wabi_binary_node) bin, from, len);
}


wabi_val
wabi_binary_sub(wabi_vm vm, wabi_val bin, wabi_val from, wabi_val len)
{

  if(wabi_val_is_forward(bin)) {
    bin = (wabi_val) (*bin & WABI_VALUE_MASK);
  }
  if(wabi_val_is_forward(from)) {
    from = (wabi_val) (*from & WABI_VALUE_MASK);
  }
  if(wabi_val_is_forward(len)) {
    len = (wabi_val) (*len & WABI_VALUE_MASK);
  }
  if(! wabi_val_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  if(!wabi_val_is_smallint(from)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  if(!wabi_val_is_smallint(len)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }

  wabi_size_t bin_len = WABI_BINARY_LENGTH((wabi_binary) bin);
  int64_t from_raw = *from & WABI_VALUE_MASK;
  int64_t len_raw = *len & WABI_VALUE_MASK;

  if(from_raw < 0
     || len_raw < 0
     || from_raw + len_raw > bin_len) {
    vm->errno = WABI_ERROR_INDEX_OUT_OF_RANGE;
    return NULL;
  }

  return (wabi_val) wabi_binary_sub_aux(vm, (wabi_binary) bin, (wabi_size_t) from_raw, (wabi_size_t) len_raw);
}


void
wabi_binary_compact_leaf(wabi_binary_leaf_t *leaf, char *dest)
{
  memcpy(dest, (char *)leaf->data_ptr, leaf->length & WABI_VALUE_MASK);
}


void
wabi_binary_compact_node(wabi_binary_node_t *node, char *dest)
{

  wabi_val left_elem;
  wabi_size_t pivot;

  left_elem = (wabi_val) node->left;
  pivot = *left_elem & WABI_VALUE_MASK;

  wabi_binary_compact_raw((wabi_val) node->left, dest);
  wabi_binary_compact_raw((wabi_val) node->right, dest + pivot);
}


void
wabi_binary_compact_raw(wabi_val bin, char *dest)
{
  return wabi_val_is_bin_leaf(bin)
    ? wabi_binary_compact_leaf((wabi_binary_leaf) bin, dest)
    : wabi_binary_compact_node((wabi_binary_node) bin, dest);
}


wabi_val
wabi_binary_compact(wabi_vm vm, wabi_val bin)
{
  if(!wabi_val_is_bin(bin)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_size_t len = WABI_BINARY_LENGTH((wabi_binary) bin);
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_binary_new(vm, len);
  if(vm->errno) return NULL;

  wabi_binary_compact_raw(bin, (char*) leaf->data_ptr);
  return (wabi_val) leaf;
}
