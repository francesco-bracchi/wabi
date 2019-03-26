#ifndef wabi_binary_h

#define wabi_binary_h

#include "wabi_object.h"
#include "wabi_err.h"

typedef struct wabi_binary_leaf_struct
{
  wabi_word_t length;
  wabi_word_t data_ptr;
} wabi_binary_leaf_t;


typedef struct wabi_binary_node_struct
{
  wabi_word_t length;
  wabi_word_t left;
  wabi_word_t right;
} wabi_binary_node_t;


#define WABI_BINARY_LEAF_SIZE 3
#define WABI_BINARY_NODE_SIZE 3

wabi_size_t
wabi_binary_word_size(wabi_size_t size);

void
wabi_binary_new(wabi_size_t size, wabi_obj *res, wabi_error *err);

void
wabi_binary_new_from_cstring(char* cstring, wabi_obj *res, wabi_error *err);

void
wabi_binary_length(wabi_obj bin, wabi_obj *res, wabi_error *err);

void
wabi_binary_concat(wabi_obj left, wabi_obj right, wabi_obj *res, wabi_error *err);

void
wabi_binary_sub(wabi_obj bin, wabi_obj from, wabi_obj len, wabi_obj* res, wabi_error *err);

// this function is useful during the GC phase
void
wabi_binary_compact(wabi_obj bin, wabi_obj *res, wabi_error *err);

// this function is useful during the GC phase
void
wabi_binary_compact_raw(wabi_obj bin, char *dest);
#endif
