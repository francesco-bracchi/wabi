/***
 * Binaries
 *
 * Basic operations:
 * 1. Length in bytes
 * 2. concatenate
 * 3. subsequence
 *
 * Binaries are represneted as "strings", altough they do no carry information
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

wabi_binary_leaf
wabi_binary_leaf_new(wabi_vm vm, wabi_size size)
{
  wabi_size word_size, *blob;
  wabi_binary_leaf leaf;

  word_size = wabi_binary_word_size(size);
  blob = (wabi_word*) wabi_vm_alloc(vm, word_size + 1);
  if(blob) {
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


wabi_binary_node
wabi_binary_node_new(wabi_vm vm, wabi_binary left, wabi_binary right) {
  wabi_word length;
  wabi_binary_node res;

  length = wabi_binary_length(left) + wabi_binary_length(right);
  res = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
  if(res) {
    res->length = length;
    res->left = (wabi_word) left;
    res->right = (wabi_word) right;
    WABI_SET_TAG(res, wabi_tag_bin_node);
  }
  return res;
}

wabi_binary
wabi_binary_sub(wabi_vm vm, wabi_binary bin, wabi_size from, wabi_size len);


wabi_binary
wabi_binary_sub_leaf(wabi_vm vm, wabi_binary_leaf leaf, wabi_size from, wabi_size len)
{
  wabi_binary_leaf new_leaf;

  new_leaf = (wabi_binary_leaf) wabi_vm_alloc(vm, WABI_BINARY_LEAF_SIZE);
  if(new_leaf) {
    new_leaf->length = len;
    new_leaf->data_ptr = leaf->data_ptr + from;
    WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);
  }
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
    return wabi_binary_sub(vm, left, from, len);
  } else if(pivot <= from) {
    return wabi_binary_sub(vm, right, from - pivot, len);
  } else {
    new_node = (wabi_binary_node) wabi_vm_alloc(vm, WABI_BINARY_NODE_SIZE);
    if(new_node) {
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

/**
 * collecting
 */

static inline void
wabi_binary_memcopy(char *dst, wabi_binary src) {
  wabi_word pivot;

  if(WABI_IS(wabi_tag_bin_leaf, src)) {
    memcpy(dst, (char *)((wabi_binary_leaf) src)->data_ptr, wabi_binary_length(src));
  } else {
    pivot = wabi_binary_length((wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst, (wabi_binary) ((wabi_binary_node) src)->left);
    wabi_binary_memcopy(dst + pivot, (wabi_binary) ((wabi_binary_node) src)->right);
  }
}


void
wabi_binary_copy_val(wabi_store store, wabi_binary src)
{
  wabi_size len, word_size;

  wabi_binary_leaf new_leaf;
  wabi_word *new_blob;
  len = wabi_binary_length((wabi_binary) src);
  word_size = wabi_binary_word_size(len);
  new_leaf = (wabi_binary_leaf) store->heap;
  store->heap += 2;
  new_blob = (wabi_word *) store->heap;
  store->heap += 1 + word_size;
  new_leaf->length = len;
  new_leaf->data_ptr = (wabi_word) (new_blob+1);
  *new_blob = 1 + word_size;
  WABI_SET_TAG(new_blob, wabi_tag_bin_blob);
  WABI_SET_TAG(new_leaf, wabi_tag_bin_leaf);

  wabi_binary_memcopy((char*)(new_blob + 1), (wabi_binary) src);
}

void
wabi_binary_collect_val(wabi_store store, wabi_binary src)
{
  // once in the new heap all binaries are of leaf type
  store->scan += WABI_BINARY_LEAF_SIZE;
}


/**
 * Hashing
 */

static inline void
wabi_binary_leaf_hash(wabi_hash_state_t *state, wabi_binary_leaf leaf)
{
  wabi_hash_step(state, (char *) leaf->data_ptr, WABI_WORD_VAL(leaf->length));
}

static inline void
wabi_binary_node_hash(wabi_hash_state_t *state, wabi_binary_node node)
{
  wabi_binary_hash(state, (wabi_binary) node->left);
  wabi_binary_hash(state, (wabi_binary) node->right);
}

void
wabi_binary_hash(wabi_hash_state_t *state, wabi_binary bin)
{
  // todo: make this if
  if(WABI_TAG(bin) == wabi_tag_bin_leaf) {
    wabi_binary_leaf_hash(state, (wabi_binary_leaf_t *) bin);
    return;
  }
  wabi_binary_node_hash(state, (wabi_binary_node_t *) bin);
}

/***
 * Comparing
 */

static inline int
wabi_binary_cmp_leaves(wabi_binary_leaf left,
                       wabi_word from_left,
                       wabi_word len_left,
                       wabi_binary_leaf right,
                       wabi_word from_right,
                       wabi_word len_right)
{
  wabi_word count = (len_left < len_right ? len_left : len_right) - 1;
  char* left_char = ((char *) left->data_ptr) + from_left;
  char* right_char = ((char *) right->data_ptr) + from_right;
  while(*left_char == *right_char && (count > 0)) {
    count--;
    left_char++;
    right_char++;
  }
  return *left_char == *right_char ? len_left - len_right : *left_char - *right_char;
}


/***
 * extensive testing
 */
static int
wabi_binary_cmp_bin(wabi_binary left,
                    wabi_word from_left,
                    wabi_word len_left,
                    wabi_binary right,
                    wabi_word from_right,
                    wabi_word len_right)
{
  if(!len_left && !len_right ) {
    return 0;
  }
  if(!len_left ) {
    return 1;
  }
  if(!len_right) {
    return -1;
  }
  if(WABI_IS(wabi_tag_bin_node, left)) {
    wabi_binary left_left = (wabi_binary) ((wabi_binary_node) left)->left;
    wabi_binary left_right = (wabi_binary) ((wabi_binary_node) left)->right;
    wabi_word pivot = wabi_binary_length(left_left);
    if(from_left >= pivot) {
      return wabi_binary_cmp_bin(left_right, from_left - pivot, len_left, right, from_right, len_right);
    }
    wabi_word left_len0 = pivot - from_left;
    if(len_left <= left_len0) {
      // is this ever visited?
      return wabi_binary_cmp_bin(left_left, from_left, len_left, right, from_right, len_right);
    }
    if(len_right <= left_len0) {
      return wabi_binary_cmp_bin(left_left, from_left, left_len0, right, from_right, len_right);
    }
    int cmp0 = wabi_binary_cmp_bin(left_left, from_left, left_len0, right, from_right, left_len0);
    if(cmp0) return cmp0;
    return wabi_binary_cmp_bin(left_right, 0, len_left - left_len0, right, from_right + left_len0, len_right - left_len0);
  }

  if(WABI_IS(wabi_tag_bin_node, right)) {
    return - wabi_binary_cmp_bin(right, from_right, len_right, left, from_left, len_left);
  }
  return wabi_binary_cmp_leaves((wabi_binary_leaf) left, from_left, len_left,
                                (wabi_binary_leaf) right, from_right, len_right);
}

int
wabi_binary_cmp(wabi_binary left, wabi_binary right)
{
  return wabi_binary_cmp_bin(left, 0, wabi_binary_length(left), right, 0, wabi_binary_length(right));
}


/**
 * Builtins
 */

//TODO: make this variadic
wabi_error_type
wabi_binary_concat_builtin(wabi_vm vm)
{
  wabi_val a, ctrl;
  wabi_binary res;

  ctrl = vm->control;

  res = (wabi_binary) wabi_binary_leaf_new_from_cstring(vm, "");
  if(! res) return wabi_error_nomem;
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    a = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);

    if(! wabi_binary_p(a))
      return wabi_error_type_mismatch;

    res = (wabi_binary) wabi_binary_node_new(vm, res, (wabi_binary) a);
    if(! res) return wabi_error_nomem;
  }

  if(*ctrl != wabi_val_nil) {
    return wabi_error_bindings;
  }
  vm->control = (wabi_val) res;
  vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
  return wabi_error_nomem;
}


inline static wabi_error_type
wabi_binary_sub_bt(wabi_vm vm, wabi_val bin, wabi_val from, wabi_val len)
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
      if(res) {
        vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
        vm->control = res;
        return wabi_error_none;
      }
      return wabi_error_nomem;
    }
  }
  return wabi_error_type_mismatch;
}

inline static wabi_error_type
wabi_binary_length_bt(wabi_vm vm, wabi_val bin)
{
  wabi_val res;
  if(wabi_binary_p(bin)) {
    res = wabi_vm_alloc(vm, 1);
    if(res) {
      *res = wabi_binary_length((wabi_binary) bin);
      WABI_SET_TAG(res, wabi_tag_fixnum);
      vm->control = res;
      vm->continuation = (wabi_val) wabi_cont_next((wabi_cont) vm->continuation);
      return wabi_error_none;
    }
    return wabi_error_nomem;
  }
  return wabi_error_type_mismatch;
}


WABI_BUILTIN_WRAP1(wabi_binary_length_builtin, wabi_binary_length_bt);
WABI_BUILTIN_WRAP3(wabi_binary_sub_builtin, wabi_binary_sub_bt);


wabi_error_type
wabi_binary_builtins(wabi_vm vm, wabi_env env)
{
  wabi_error_type res;

  res = WABI_DEFN(vm, env, "blen", "blen", wabi_binary_length_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, "bcat", "bcat", wabi_binary_concat_builtin);
  if(res) return res;
  res = WABI_DEFN(vm, env, "bsub", "bsub", wabi_binary_sub_builtin);
  return res;
}
