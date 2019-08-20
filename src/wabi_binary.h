#ifndef wabi_binary_h

#define wabi_binary_h

#include "wabi_value.h"
#include "wabi_store.h"

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


#define WABI_BINARY_LEAF_SIZE 3
#define WABI_BINARY_NODE_SIZE 3


wabi_binary_leaf
wabi_binary_leaf_new(wabi_store store, wabi_size size);


wabi_binary_leaf
wabi_binary_leaf_new_from_cstring(wabi_store store, char* cstring);


wabi_size
wabi_binary_length(wabi_binary bin);


wabi_binary_node
wabi_binary_concat(wabi_store store, wabi_binary left, wabi_binary right);


wabi_binary
wabi_binary_sub(wabi_store store, wabi_binary bin, wabi_size from, wabi_size len);


void
wabi_binary_compact(wabi_binary bin, char *dest);


#endif
