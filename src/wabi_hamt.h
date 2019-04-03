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


typedef struct wabi_hamt_table_struct
{
  wabi_word_t low;
  wabi_word_t up;
} wabi_hamt_table_t;


typedef wabi_hamt_table_t* wabi_hamt_table;


typedef short unsigned int wabi_hamt_index;

#define WABI_HAMT_BF 64
#define WABI_HAMT_SIZE 2


#define WABI_POPCNT(v) __builtin_popcountl(v)

#define wabi_hamt_bit_set(bitmap, index) ((bitmap) | (1 << (64 - (index))))
#define wabi_hamt_bit_get(bitmap, index) (((bitmap) >> (64 - (index))) & 1)
#define wabi_hamt_table_offset(bitmap, index) (WABI_POPCNT((bitmap) >> (index)))
#define wabi_hamt_index(hash, offset) (((hash) >> (50 - (offset))) & 0x3F)
#define wabi_hamt_is_map(entry) wabi_obj_is_hamt_map((wabi_obj) entry)


wabi_obj
wabi_hamt_empty(wabi_vm vm);


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key);


wabi_obj
wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val);


/* wabi_hamt_kw */
/* wabi_hamt_get_kw_by_hash(wabi_hamt_map map, wabi_word_t hash); */


/* inline wabi_hamt_kw */
/* wabi_hamt_get_entry_raw(wabi_hamt_map map, wabi_obj key) */
/* { */
/*   wabi_word_t hash = wabi_hash_raw(key); */
/*   return wabi_hamt_get_kw_by_hash(map, hash); */
/* } */

#endif
