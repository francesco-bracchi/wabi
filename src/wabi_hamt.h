#ifndef wabi_hamt_h

#define wabi_hamt_h

#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_hash.h"


typedef struct wabi_hamt_entry_struct
{
  wabi_word_t value;
  wabi_word_t key;
} wabi_hamt_entry_t;


typedef wabi_hamt_entry_t* wabi_hamt_entry;


typedef struct wabi_hamt_map_struct
{
  wabi_word_t table;
  wabi_word_t bitmap;
} wabi_hamt_map_t;


typedef wabi_hamt_map_t* wabi_hamt_map;


typedef union wabi_hamt_table_struct
{
  wabi_hamt_entry_t entry;
  wabi_hamt_map_t map;
} wabi_hamt_table_t;


typedef wabi_hamt_table_t* wabi_hamt_table;


typedef int wabi_hamt_index;


#define WABI_HAMT_SIZE 2

#define WABI_POPCNT(v) __builtin_popcountl(v)

wabi_obj
wabi_hamt_empty(wabi_vm vm);


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key);


wabi_obj
wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val);

#endif
