#define wabi_binary_c

#define WABI_BINARY_LEAF_SIZE 3

#include <stdio.h>

#include <string.h>
#include "wabi_object.h"
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

void
wabi_blob_new(wabi_size_t size, wabi_obj *res, wabi_error *err)
{
  size ++;
  wabi_mem_allocate(size, res, err);
  **res = size | WABI_TAG_BIN_BLOB;
  *res = *res + 1;
}

void
wabi_binary_new(wabi_size_t size, wabi_obj *res, wabi_error *err)
{
  wabi_size_t word_size;
  wabi_word_t *blob;
  wabi_binary_leaf_t *leaf;

  wabi_mem_allocate(WABI_BINARY_LEAF_SIZE, res, err);
  if(*err != WABI_ERROR_NONE) return;

  word_size = wabi_binary_word_size(size);
  wabi_mem_allocate(word_size + 1, &blob, err);
  *blob = (wabi_word_t) ((word_size + 1) | WABI_TAG_BIN_BLOB);
  leaf = (wabi_binary_leaf_t *) *res;
  leaf->length = size | WABI_TAG_BIN_LEAF;
  leaf->data_ptr = (wabi_word_t) (blob + 1);

}

void
wabi_binary_new_from_cstring(char* cstring, wabi_obj *res, wabi_error *err)
{
  wabi_size_t size;
  wabi_binary_leaf_t *leaf;

  size = strlen(cstring);
  wabi_binary_new(size, res, err);
  if(*err != WABI_ERROR_NONE) return;

  leaf = (wabi_binary_leaf_t *) *res;
  memcpy((char *) leaf->data_ptr, cstring, size);
}

void
wabi_binary_length(wabi_obj bin, wabi_obj *res, wabi_error *err)
{
  int64_t l;

  switch(wabi_obj_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
  case WABI_TAG_BIN_NODE:
    l = (int64_t) *bin & WABI_VALUE_MASK;
    wabi_smallint(l, res, err);
    return;
  default:
    *err = WABI_ERROR_TYPE_MISMATCH;
  }
}

void
wabi_binary_concat(wabi_obj left, wabi_obj right, wabi_obj *res, wabi_error *err)
{
  if((wabi_obj_is_bin(left)) && (wabi_obj_is_bin(right))) {
    wabi_mem_allocate(WABI_BINARY_NODE_SIZE, res, err);
    wabi_binary_node_t *node = (wabi_binary_node_t *) *res;
    node->length = ((*left & WABI_VALUE_MASK) + (*right & WABI_VALUE_MASK)) | WABI_TAG_BIN_NODE;
    node->left = (wabi_word_t) left;
    node->right = (wabi_word_t) right;
    return;
  }

  *err = WABI_ERROR_TYPE_MISMATCH;
  return;
}

void
wabi_binary_sub_aux(wabi_obj bin, wabi_size_t from, wabi_size_t len, wabi_obj *res, wabi_error *err);

void
wabi_binary_sub_leaf(wabi_binary_leaf_t *leaf, wabi_size_t from, wabi_size_t len, wabi_obj *res, wabi_error *err)
{
  wabi_binary_leaf_t *new_leaf;
  wabi_mem_allocate(WABI_BINARY_LEAF_SIZE, res, err);
  if(*err != WABI_ERROR_NONE) return;

  new_leaf = (wabi_binary_leaf_t *) *res;
  new_leaf->length = len | WABI_TAG_BIN_LEAF;
  new_leaf->data_ptr = leaf->data_ptr + from;
}

void
wabi_binary_sub_node(wabi_binary_node_t *node, wabi_size_t from, wabi_size_t len, wabi_obj *res, wabi_error *err)
{
  wabi_binary_node_t *new_node;
  wabi_obj left_elem, new_left, new_right;
  wabi_size_t pivot;

  left_elem = (wabi_obj) node->left;
  pivot = *left_elem & WABI_VALUE_MASK;

  if(pivot > from + len) {
    wabi_binary_sub_aux(left_elem, from, len, res, err);
  } else if(pivot <= from) {
    wabi_binary_sub_aux((wabi_obj) node->right, from - pivot, len, res, err);
  } else {
    wabi_binary_sub_aux((wabi_obj) node->left, from, pivot - from, &new_left, err);
    if(*err != WABI_ERROR_NONE) return;

    wabi_binary_sub_aux((wabi_obj) node->right, 0, len + from - pivot, &new_right, err);
    if(*err != WABI_ERROR_NONE) return;

    wabi_mem_allocate(WABI_BINARY_NODE_SIZE, res, err);
    if(*err != WABI_ERROR_NONE) return;

    new_node = (wabi_binary_node_t *) *res;
    new_node->length = len | WABI_TAG_BIN_NODE;
    new_node->left = (wabi_word_t) new_left;
    new_node->right = (wabi_word_t) new_right;
  }
}

void
wabi_binary_sub_aux(wabi_obj bin, wabi_size_t from, wabi_size_t len, wabi_obj *res, wabi_error *err)
{
  switch(wabi_obj_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
    wabi_binary_sub_leaf((wabi_binary_leaf_t *) bin, from, len, res, err);
    break;
  default:
    wabi_binary_sub_node((wabi_binary_node_t *) bin, from, len, res, err);
  }
}

void
wabi_binary_sub(wabi_obj bin, wabi_obj from, wabi_obj len, wabi_obj* res, wabi_error *err)
{
  wabi_size_t bin_len_s;
  int64_t from_s, len_s;

  if(! wabi_obj_is_bin(bin)) {
    *err = WABI_ERROR_TYPE_MISMATCH;
    return;
  }
  if(!wabi_obj_is_smallint(from)) {
    *err = WABI_ERROR_TYPE_MISMATCH;
    return;
  }
  if(!wabi_obj_is_smallint(len)) {
    *err = WABI_ERROR_TYPE_MISMATCH;
    return;
  }

  bin_len_s = *bin & WABI_VALUE_MASK;
  from_s = *from & WABI_VALUE_MASK;
  len_s = *len & WABI_VALUE_MASK;

  if(from_s < 0 || len_s < 0 || from_s + len_s > bin_len_s) {
    *err = WABI_ERROR_INDEX_OUT_OF_RANGE;
  }
  wabi_binary_sub_aux(bin, (wabi_size_t) from_s, (wabi_size_t) len_s, res, err);
}


void
wabi_binary_compact_leaf(wabi_binary_leaf_t *leaf, char *dest)
{
  memcpy(dest, (char *)leaf->data_ptr, leaf->length & WABI_VALUE_MASK);
}

void
wabi_binary_compact_node(wabi_binary_node_t *node, char *dest)
{

  wabi_obj left_elem;
  wabi_size_t pivot;

  left_elem = (wabi_obj) node->left;
  pivot = *left_elem & WABI_VALUE_MASK;

  wabi_binary_compact((wabi_obj) node->left, dest);
  wabi_binary_compact((wabi_obj) node->right, dest + pivot);
}

void
wabi_binary_compact(wabi_obj bin, char *dest)
{
  switch(wabi_obj_tag(bin)) {
  case WABI_TAG_BIN_LEAF:
    wabi_binary_compact_leaf((wabi_binary_leaf_t *) bin, dest);
    break;
  default:
    wabi_binary_compact_node((wabi_binary_node_t *) bin, dest);
  }
}

// to be removed
void
wabi_binary_compact_cmd(wabi_obj bin, wabi_obj *res, wabi_error *err)
{
  wabi_size_t len;
  wabi_binary_leaf_t *leaf;

  if(! wabi_obj_is_bin(bin)) {
    *err = WABI_ERROR_TYPE_MISMATCH;
    return;
  }
  len = *bin & WABI_VALUE_MASK;
  wabi_binary_new(len, res, err);
  if(*err != WABI_ERROR_NONE) return;
  leaf = (wabi_binary_leaf_t *) *res;
  wabi_binary_compact(bin, (char *) leaf->data_ptr);
}
