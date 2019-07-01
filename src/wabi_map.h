#ifndef wabi_map_h

#define wabi_map_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_hash.h"
#include "wabi_cmp.h"

#define WABI_MAP_SIZE 2
#define WABI_MAP_ARRAY_LIMIT 32
#define WABI_MAP_INITIAL_OFFSET 50
#define WABI_MAP_OFFSET_INCREMENT 6
#define WABI_MAP_BYTE_SIZE WABI_WORD_SIZE * WABI_MAP_SIZE
#define WABI_MAP_ITER_STACK_SIZE 10

typedef struct wabi_map_array_struct
{
  wabi_word_t table;
  wabi_word_t size;
} wabi_map_array_t;

typedef wabi_map_array_t* wabi_map_array;

typedef struct wabi_map_entry_struct
{
  wabi_word_t value;
  wabi_word_t key;
} wabi_map_entry_t;

typedef wabi_map_entry_t* wabi_map_entry;

typedef struct wabi_map_hash_struct
{
  wabi_word_t table;
  wabi_word_t bitmap;
} wabi_map_hash_t;

typedef wabi_map_hash_t* wabi_map_hash;

typedef union wabi_map_union
{
  wabi_map_array_t array;
  wabi_map_entry_t entry;
  wabi_map_hash_t  hash;
} wabi_map_t;

typedef wabi_map_t* wabi_map;

typedef int wabi_map_index;

typedef struct wabi_map_iter_frame_struct {
  wabi_map map;
  wabi_word_t pos;
} wabi_map_iter_frame_t;

typedef wabi_map_iter_frame_t* wabi_map_iter_frame;

typedef struct wabi_map_iter_struct {
  wabi_map_iter_frame_t stack[WABI_MAP_ITER_STACK_SIZE];
  int top;
} wabi_map_iter_t;

typedef wabi_map_iter_t* wabi_map_iter;

#define WABI_POPCNT(v) __builtin_popcountl(v)
#define WABI_MAP_ARRAY_SIZE(map) ((map)->size)
#define WABI_MAP_ARRAY_TABLE(map) ((wabi_map) ((map)->table & WABI_VALUE_MASK))

#define WABI_MAP_HASH_TABLE(map) ((wabi_map) ((map)->table & WABI_VALUE_MASK))
#define WABI_MAP_HASH_BITMAP(map) ((map)->bitmap)
#define WABI_MAP_HASH_INDEX(hash, h_pos) (((hash) >> h_pos) & 0x3F)

#define WABI_MAP_BITMAP_OFFSET(bitmap, index) ((index) ? WABI_POPCNT((bitmap) << (64 - (index))) : 0)
#define WABI_MAP_BITMAP_COUNT(bitmap) WABI_POPCNT(bitmap)
#define WABI_MAP_BITMAP_CONTAINS(bitmap, index) (((bitmap) >> (index)) & 1LU)

#define WABI_MAP_ENTRY_KEY(entry) ((wabi_val) ((entry)->key))
#define WABI_MAP_ENTRY_VALUE(entry) ((wabi_val) ((entry)->value & WABI_VALUE_MASK))

wabi_val
wabi_map_assoc(wabi_vm vm,
               wabi_val map,
               wabi_val key,
               wabi_val value);

wabi_map
wabi_map_assoc_raw(wabi_vm vm,
                   wabi_map map,
                   wabi_val key,
                   wabi_val value);

wabi_map
wabi_map_dissoc_raw(wabi_vm vm,
                    wabi_map map,
                    wabi_val key);

wabi_val
wabi_map_dissoc(wabi_vm vm,
                wabi_val map,
                wabi_val key);

wabi_val
wabi_map_empty(wabi_vm vm);


wabi_val
wabi_map_get_raw(wabi_map map,
                 wabi_val key);


wabi_val
wabi_map_get(wabi_vm vm,
             wabi_val map,
             wabi_val key);


void
wabi_map_iterator_init(wabi_map_iter iter,
                       wabi_map map);


wabi_map_entry
wabi_map_iterator_current(wabi_map_iter iter);


void
wabi_map_iterator_next(wabi_map_iter iter);


wabi_word_t
wabi_map_length_raw(wabi_map map);


wabi_val
wabi_map_length(wabi_vm vm, wabi_val map);

#endif
