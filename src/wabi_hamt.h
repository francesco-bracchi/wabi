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
#define MAP_BITMAP(map) ((map)->bitmap)
#define MAP_TABLE(map) ((wabi_hamt_table) ((map)->table & WABI_VALUE_MASK))
#define ENTRY_KEY(entry) ((entry)->key & WABI_VALUE_MASK)
#define ENTRY_VALUE(entry) ((entry)->value & WABI_VALUE_MASK)
#define BITMAP_OFFSET(bitmap, index) WABI_POPCNT((bitmap) << (64 - (index)) & 0xFFFFFFFFFFFFFFFE)
#define BITMAP_FOUND(bitmap, index) (((bitmap) >> (index)) & 1)
#define BITMAP_SIZE(bitmap) WABI_POPCNT(bitmap)
#define HASH_INDEX(hash, h_pos) (((hash) >> h_pos) & 0x3F)


wabi_obj
wabi_hamt_empty(wabi_vm vm);


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key);



wabi_obj
wabi_hamt_assoc(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val);



int64_t
wabi_hamt_length_raw(wabi_hamt_map map);


wabi_obj
wabi_hamt_get_raw(wabi_obj map, wabi_obj key);


wabi_obj
wabi_hamt_length(wabi_vm vm, wabi_obj map);

#endif
