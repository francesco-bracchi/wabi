#ifndef wabi_binary_h

#define wabi_binary_h

#include "wabi_value.h"
#include "wabi_err.h"
#include "wabi_vm.h"

typedef struct wabi_binary_leaf_struct
{
  wabi_word_t length;
  wabi_word_t data_ptr;
} wabi_binary_leaf_t;

typedef wabi_binary_leaf_t* wabi_binary_leaf;

typedef struct wabi_binary_node_struct
{
  wabi_word_t length;
  wabi_word_t left;
  wabi_word_t right;
} wabi_binary_node_t;

typedef wabi_binary_node_t* wabi_binary_node;

typedef union wabi_binary_union
{
  wabi_word_t length;
  wabi_binary_leaf_t leaf;
  wabi_binary_node_t node;
} wabi_binary_t;

typedef wabi_binary_t* wabi_binary;


#define WABI_BINARY_LEAF_SIZE 3
#define WABI_BINARY_NODE_SIZE 3

#define WABI_BINARY_LENGTH(bin) ((bin)->length & WABI_VALUE_MASK)
#define WABI_BINARY_NODE_LEFT(bin) ((wabi_binary) ((bin)->left))
#define WABI_BINARY_NODE_RIGHT(bin) ((wabi_binary) ((bin)->right))
#define WABI_BINARY_DATA_PTR(bin) ((char*) ((bin)->data_ptr))

wabi_size_t
wabi_binary_word_size(wabi_size_t size);


wabi_val
wabi_binary_new(wabi_vm vm, wabi_size_t size);


wabi_val
wabi_binary_new_from_cstring(wabi_vm vm, char* cstring);


wabi_val
wabi_binary_length(wabi_vm vm, wabi_val bin);


wabi_val
wabi_binary_concat(wabi_vm vm, wabi_val left, wabi_val right);


wabi_val
wabi_binary_sub(wabi_vm vm, wabi_val bin, wabi_val from, wabi_val len);


// this function is useful during the GC phase
wabi_val
wabi_binary_compact(wabi_vm vm, wabi_val bin);


// this function is useful during the GC phase
void
wabi_binary_compact_raw(wabi_val bin, char *dest);


wabi_size_t
wabi_binary_length_raw(wabi_binary bin);

#endif
