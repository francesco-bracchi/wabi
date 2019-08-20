#define wabi_binary_c

#include "wabi_value.h"
#include "wabi_word.h"
#include "wabi_store.h"
#include "wabi_binary.h"
#include <string.h>

wabi_size
wabi_binary_word_size(wabi_size size)
{
  wabi_size word_size;
  word_size = size >> 2;
  if(size & 3) word_size++;
  return word_size;
}


wabi_val
wabi_blob_new(wabi_store store, wabi_size size)
{
  size ++;
  wabi_val res = wabi_store_heap_alloc(store, size);
  if(!res) return NULL;
  *res = size;
  WABI_SET_TAG(res, wabi_tag_bin_blob);
  return res + 1;
}

wabi_binary_leaf
wabi_binary_leaf_new(wabi_store store, wabi_size size)
{
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_store_heap_alloc(store, WABI_BINARY_LEAF_SIZE);
  if(! leaf) return NULL;

  wabi_size word_size = wabi_binary_word_size(size);
  wabi_word* blob = (wabi_word*) wabi_store_heap_alloc(store, word_size + 1);
  if(! blob) return  NULL;

  leaf->length = size;
  leaf->data_ptr = (wabi_word) blob;
  WABI_SET_TAG(leaf, wabi_tag_bin_leaf);
  return leaf;
}

wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(wabi_store store, char* cstring)
{
  wabi_size size = strlen(cstring);
  wabi_binary_leaf leaf = wabi_binary_leaf_new(store, size);
  if(leaf) {
  memcpy((char *) leaf->data_ptr, cstring, size);
  return leaf;
  }
  return NULL;
}

wabi_size
wabi_binary_length(wabi_binary bin)
{
  return WABI_WORD_VAL(bin->length);
}


wabi_binary_node
wabi_binary_concat(wabi_store store, wabi_binary left, wabi_binary right)
{
  wabi_binary_node node = (wabi_binary_node) wabi_store_heap_alloc(store, WABI_BINARY_NODE_SIZE);
  if(! node) return NULL;

  wabi_size length = wabi_binary_length(left) + wabi_binary_length(right);
  node->length = length;
  node->left = (wabi_word) left;
  node->right = (wabi_word) right;
  WABI_SET_TAG(node, wabi_tag_bin_node);
  return node;
}

wabi_binary
wabi_binary_sub_aux(wabi_store store, wabi_binary bin, wabi_size from, wabi_size len);


wabi_binary
wabi_binary_sub_leaf(wabi_store store, wabi_binary_leaf leaf, wabi_size from, wabi_size len)
{
  wabi_binary_leaf new_leaf = (wabi_binary_leaf) wabi_store_heap_alloc(store, WABI_BINARY_LEAF_SIZE);
  if(! new_leaf) return NULL;

  new_leaf->length = len;
  new_leaf->data_ptr = leaf->data_ptr + from;
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);
  return (wabi_binary) new_leaf;
}

wabi_binary
wabi_binary_sub_node(wabi_store store, wabi_binary_node node, wabi_size from, wabi_size len)
{
  wabi_binary left = (wabi_binary) node->left;
  wabi_binary right = (wabi_binary) node->right;
  wabi_size pivot = wabi_binary_length(left);

  if(pivot > from + len) {
    return wabi_binary_sub_aux(store, left, from, len);
  } else if(pivot <= from) {
    return wabi_binary_sub_aux(store, right, from - pivot, len);
  } else {
    wabi_binary_node new_node = (wabi_binary_node) wabi_store_heap_alloc(store, WABI_BINARY_NODE_SIZE);
    if(! new_node) return NULL;

    wabi_binary new_left = wabi_binary_sub_aux(store, left, from, pivot - from);
    if(! new_left) return NULL;

    wabi_binary new_right = wabi_binary_sub_aux(store, right, 0, len + from - pivot);
    if(! new_right) return NULL;

    new_node->length = len;
    new_node->left = (wabi_word) new_left;
    new_node->right = (wabi_word) new_right;
    WABI_SET_TAG(new_node, wabi_tag_bin_node);
    return (wabi_binary) new_node;
  }
}

wabi_binary
wabi_binary_sub_aux(wabi_store store, wabi_binary bin, wabi_size from, wabi_size len)
{
  return WABI_IS(wabi_tag_bin_leaf, bin)
    ? wabi_binary_sub_leaf(store, (wabi_binary_leaf) bin, from, len)
    : wabi_binary_sub_node(store, (wabi_binary_node) bin, from, len);
}


wabi_binary
wabi_binary_sub(wabi_store store, wabi_binary bin, wabi_size from, wabi_size len)
{
  wabi_size bin_len = wabi_binary_length(bin);
  if(from < 0 || len < 0 || from + len > bin_len)  return NULL;

  return (wabi_binary) wabi_binary_sub_aux(store, bin, from, len);
}


void
wabi_binary_compact(wabi_binary bin, char *dest);

void
wabi_binary_compact_leaf(wabi_binary_leaf_t *leaf, char *dest)
{
  memcpy(dest, (char *)leaf->data_ptr, wabi_binary_length((wabi_binary) leaf));
}


void
wabi_binary_compact_node(wabi_binary_node_t *node, char *dest)
{

  wabi_binary left_elem;
  wabi_size pivot;

  left_elem = (wabi_binary) node->left;
  pivot = wabi_binary_length(left_elem);

  wabi_binary_compact((wabi_binary) node->left, dest);
  wabi_binary_compact((wabi_binary) node->right, dest + pivot);
}


void
wabi_binary_compact(wabi_binary bin, char *dest)
{
  return WABI_IS(wabi_tag_bin_leaf, bin)
    ? wabi_binary_compact_leaf((wabi_binary_leaf) bin, dest)
    : wabi_binary_compact_node((wabi_binary_node) bin, dest);
}
