#ifndef wabi_hamt_h

#define wabi_hamt_h

#include "wabi_object.h"
#include "wabi_vm.h"


typedef struct wabi_hamt_pair_struct
{
  wabi_word_t value;
  wabi_word_t key;
} wabi_hamt_pair_t;


typedef wabi_hamt_pair_t* wabi_hamt_pair;


typedef struct wabi_hamt_map_struct
{
  wabi_word_t table;
  wabi_word_t bitmap;
} wabi_hamt_map_t;

typedef wabi_hamt_map_t* wabi_hamt_map;

typedef union wabi_hamt_entry_union
{
  wabi_hamt_pair_t pair;
  wabi_hamt_map_t map;
} wabi_hamt_entry_t;

typedef wabi_hamt_entry_t* wabi_hamt_entry;

#define WABI_HAMT_BF 64
#define WABI_HAMT_SIZE 2

wabi_obj
wabi_hamt_empty(wabi_vm vm);
#endif
