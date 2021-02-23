#ifndef wabi_binary_h

#define wabi_binary_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_env.h"

typedef struct wabi_binary_leaf_struct
{
  wabi_word length;
  wabi_word data_ptr;
} wabi_binary_leaf_t;

typedef wabi_binary_leaf_t* wabi_binary_leaf;

typedef struct wabi_binary_node_struct
{
  wabi_word length;
  wabi_word left;
  wabi_word right;
} wabi_binary_node_t;

typedef wabi_binary_node_t* wabi_binary_node;

typedef union wabi_binary_union
{
  wabi_word length;
  wabi_binary_leaf_t leaf;
  wabi_binary_node_t node;
} wabi_binary_t;

typedef wabi_binary_t* wabi_binary;


#define WABI_BINARY_LEAF_SIZE wabi_sizeof(wabi_binary_leaf_t)
#define WABI_BINARY_NODE_SIZE wabi_sizeof(wabi_binary_node_t)


static inline wabi_size
wabi_binary_word_size(const wabi_size size)
{
  wabi_size word_size;
  word_size = size >> 2;
  if(size & 3) word_size++;
  return word_size;
}


static inline wabi_size
wabi_binary_length(const wabi_binary bin)
{
  return WABI_WORD_VAL(bin->length);
}


static inline int
wabi_is_binary(const wabi_val v) {
  switch(WABI_TAG(v)) {
  case wabi_tag_bin_leaf:
  case wabi_tag_bin_node:
    return 1;
  }
  return 0;
}


wabi_binary_leaf
wabi_binary_leaf_new(const wabi_vm vm, const wabi_size size);


wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(const wabi_vm vm, const char* cstring);


wabi_binary_node
wabi_binary_concat(const wabi_vm vm, const wabi_binary left, const wabi_binary right);


wabi_binary
wabi_binary_sub(const wabi_vm vm, const wabi_binary bin, const wabi_size from, const wabi_size len);

void
wabi_binary_builtins(const wabi_vm vm, const wabi_env env);

char*
wabi_binary_to_cstring(const wabi_vm vm,
                       const wabi_binary bin);

#endif
