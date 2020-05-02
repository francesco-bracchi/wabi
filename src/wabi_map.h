#ifndef wabi_map_h
#define wabi_map_h

#include "wabi_vm.h"
#include "wabi_value.h"
#include "wabi_hash.h"
#include "wabi_cmp.h"
#include "wabi_store.h"

#define WABI_MAP_ITER_STACK_SIZE 10

typedef struct wabi_map_array_struct
{
  wabi_word table;
  wabi_word size;
} wabi_map_array_t;

typedef wabi_map_array_t* wabi_map_array;

typedef struct wabi_map_entry_struct
{
  wabi_word value;
  wabi_word key;
} wabi_map_entry_t;

typedef wabi_map_entry_t* wabi_map_entry;

typedef struct wabi_map_hash_struct
{
  wabi_word table;
  wabi_word bitmap;
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
  wabi_word pos;
} wabi_map_iter_frame_t;

typedef wabi_map_iter_frame_t* wabi_map_iter_frame;

typedef struct wabi_map_iter_struct {
  wabi_map_iter_frame_t stack[WABI_MAP_ITER_STACK_SIZE];
  int top;
} wabi_map_iter_t;

typedef wabi_map_iter_t* wabi_map_iter;

#define WABI_MAP_SIZE wabi_sizeof(wabi_map_t)
#define WABI_MAP_ARRAY_LIMIT 32
#define WABI_MAP_INITIAL_OFFSET 53
#define WABI_MAP_OFFSET_INCREMENT 6
#define WABI_MAP_BYTE_SIZE WABI_WORD_SIZE * WABI_MAP_SIZE


#define WABI_POPCNT(v) __builtin_popcountl(v)
#define WABI_MAP_ARRAY_SIZE(map) ((map)->size)
#define WABI_MAP_ARRAY_TABLE(map) ((wabi_map) WABI_WORD_VAL((map)->table))

#define WABI_MAP_HASH_TABLE(map) ((wabi_map) WABI_WORD_VAL((map)->table))
#define WABI_MAP_HASH_BITMAP(map) ((map)->bitmap)
#define WABI_MAP_HASH_INDEX(hash, h_pos) (((hash) >> h_pos) & 0x3F)

#define WABI_MAP_BITMAP_OFFSET(bitmap, index) ((index) ? WABI_POPCNT((bitmap) << (64 - (index))) : 0)
#define WABI_MAP_BITMAP_COUNT(bitmap) WABI_POPCNT(bitmap)
#define WABI_MAP_BITMAP_CONTAINS(bitmap, index) (((bitmap) >> (index)) & 1LU)

#define WABI_MAP_ENTRY_KEY(entry) ((wabi_val) ((entry)->key))
#define WABI_MAP_ENTRY_VALUE(entry) ((wabi_val) WABI_WORD_VAL((entry)->value))


wabi_map
wabi_map_assoc(wabi_vm vm,
               wabi_map map,
               wabi_val key,
               wabi_val value);

wabi_map
wabi_map_dissoc(wabi_vm vm,
                wabi_map map,
                wabi_val key);


wabi_map
wabi_map_empty(wabi_vm vm);


wabi_val
wabi_map_get(wabi_map map,
             wabi_val key);

void
wabi_map_iterator_init(wabi_map_iter iter,
                       wabi_map map);


wabi_map_entry
wabi_map_iterator_current(wabi_map_iter iter);


void
wabi_map_iterator_next(wabi_map_iter iter);


wabi_word
wabi_map_length(wabi_map map);


void
wabi_map_hash_(wabi_hash_state state, wabi_map map);


int
wabi_map_cmp(wabi_map left, wabi_map right);


wabi_error_type
wabi_map_builtins(wabi_vm vm, wabi_env env);


static inline void
wabi_map_array_copy_val(wabi_vm vm, wabi_map_array map)
{
  wabi_copy_val_size(vm, (wabi_val) map, WABI_MAP_SIZE);
}

static inline void
wabi_map_entry_copy_val(wabi_vm vm, wabi_map_entry map)
{
  wabi_copy_val_size(vm, (wabi_val) map, WABI_MAP_SIZE);
}

static inline void
wabi_map_hash_copy_val(wabi_vm vm, wabi_map_hash map)
{
  wabi_copy_val_size(vm, (wabi_val) map, WABI_MAP_SIZE);
}

static inline void
wabi_map_entry_collect_val(wabi_vm vm, wabi_map_entry entry)
{
  wabi_collect_val_size(vm, (wabi_val) entry, WABI_MAP_SIZE);
}


static inline void
wabi_map_array_collect_val(wabi_vm vm, wabi_map_array array)
{
  wabi_word size;
  size = array->size;

  wordcopy(vm->stor.heap, (wabi_word*) WABI_WORD_VAL(array->table), wabi_sizeof(wabi_map_entry_t) * size);
  array->table = (wabi_word) vm->stor.heap;
  vm->stor.heap += wabi_sizeof(wabi_map_entry_t) * size;
  WABI_SET_TAG(array, wabi_tag_map_array);
  vm->stor.scan += wabi_sizeof(wabi_map_array_t);
}


static inline void
wabi_map_hash_collect_val(wabi_vm vm, wabi_map_hash map)
{
  wabi_word size;
  size = WABI_MAP_BITMAP_COUNT(map->bitmap);

  wordcopy(vm->stor.heap, (wabi_word*) WABI_WORD_VAL(map->table), wabi_sizeof(wabi_map_entry_t) * size);
  map->table = (wabi_word) vm->stor.heap;
  vm->stor.heap += wabi_sizeof(wabi_map_entry_t) * size;
  WABI_SET_TAG(map, wabi_tag_map_hash);
  vm->stor.scan += wabi_sizeof(wabi_map_hash_t);
}

#endif
