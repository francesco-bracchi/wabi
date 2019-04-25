/**
 * 1. an empty map is an array map 0 length
 * 3. a map with less than `WABI_MAP_ARRAY_LIMIT` elements is an array map
 * 4. a map with more than `WABI_MAP_ARRAY_LIMIT` elements is a hash map
 * 5. Each node of a hash map can be:
 *    a) an entry
 *    b) an array map
 *    c) a hash map
 * 6. if hash is completely consumed, (i.e. too deep) use only array maps
 */

#define wabi_map_c

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_hash.h"
#include "wabi_atomic.h"
#include "wabi_cmp.h"
#include "wabi_map.h"

/**
 * Assoc operation
 */
wabi_map
wabi_map_hash_assoc_rec(wabi_vm vm,
                        wabi_map_hash map,
                        wabi_map_entry entry,
                        wabi_word_t hash,
                        int hash_offset);


wabi_map
wabi_map_array_assoc_rec(wabi_vm vm,
                         wabi_map_array map,
                         wabi_map_entry entry,
                         wabi_word_t hash,
                         int hash_offset);


wabi_map
wabi_map_assoc_rec(wabi_vm vm,
                   wabi_map map,
                   wabi_map_entry entry,
                   wabi_word_t hash,
                   int hash_offset);


wabi_map_hash
wabi_map_array_promote(wabi_vm vm,
                       wabi_map_array map,
                       int hash_offset)
{
  wabi_map_hash new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE);
  if(vm->errno) return NULL;
  new_map->bitmap = 0UL;
  new_map->table = 0UL;

  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_map limit = table + WABI_MAP_ARRAY_SIZE(map);
  wabi_map row = table;
  while(row < limit) {
    wabi_val key = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    wabi_word_t hash = wabi_hash_raw(key);
    new_map = (wabi_map_hash) wabi_map_hash_assoc_rec(vm, new_map, (wabi_map_entry) row, hash, hash_offset);
    if(vm->errno) return NULL;
    row++;
  }
  return new_map;
}


wabi_map
wabi_map_array_assoc_rec(wabi_vm vm,
                         wabi_map_array map,
                         wabi_map_entry entry,
                         wabi_word_t hash,
                         int hash_offset)
{
  wabi_val key = (wabi_val) WABI_MAP_ENTRY_KEY(entry);
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  wabi_map row = table;
  wabi_map limit = table + size;

  // key lookup, TODO: insert ordered
  while(row < limit) {
    wabi_val key0 = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    if(wabi_eq_raw(key, key0)) {
      // if found replace the entry
      wabi_word_t pos = row - table;
      wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 1));
      if(vm->errno) return NULL;

      wabi_map new_table = (wabi_map) (new_map + 1);
      memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
      *(new_table + pos) = (wabi_map_t) *entry;

      new_map->size = size;
      new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
      return (wabi_map) new_map;
    }
    row++;
  }
  // if not found and the hash is not completely exahausted
  if(hash_offset > 0 && size >= WABI_MAP_ARRAY_LIMIT) {
    wabi_map_hash hash_map = wabi_map_array_promote(vm, map, hash_offset);
    if(vm->errno) return NULL;
    return wabi_map_hash_assoc_rec(vm, hash_map, entry, hash, hash_offset);
  }
  // if not found and hash is exhausted
  wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 2));
  if(vm->errno) return NULL;

  wabi_map new_table = (wabi_map) (new_map + 1);
  memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
  *(new_table + size) = (wabi_map_t) *entry;

  new_map->size = size + 1;
  new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
  return (wabi_map) new_map;
}


wabi_map
wabi_map_hash_assoc_rec(wabi_vm vm,
                        wabi_map_hash map,
                        wabi_map_entry entry,
                        wabi_word_t hash,
                        int hash_offset)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_word_t index = WABI_MAP_HASH_INDEX(hash, hash_offset);
  wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    wabi_map row = table + offset;

    wabi_map_hash new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 1));
    if(vm->errno) return NULL;

    wabi_map sub_map = wabi_map_assoc_rec(vm, row, entry, hash, hash_offset - 6);
    if(vm->errno) return NULL;

    wabi_map new_table = (wabi_map) (new_map + 1);
    memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
    *(new_table + offset) = *sub_map;
    new_map->table = ((wabi_word_t) new_table) | WABI_TAG_MAP_HASH;
    new_map->bitmap = bitmap;
    return (wabi_map) new_map;
  }
  wabi_map_hash new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 2));
  if(vm->errno) return NULL;
  wabi_map new_table = (wabi_map) (new_map + 1);

  memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
  *(new_table + offset) = (wabi_map_t) *entry;
  memcpy(new_table + offset + 1, table + offset, WABI_MAP_BYTE_SIZE * (size - offset));

  new_map->bitmap = bitmap & (1UL << index);
  new_map->table = ((wabi_word_t) new_table) | WABI_TAG_MAP_HASH;
  return (wabi_map) new_map;
}


wabi_map
wabi_map_assoc_rec(wabi_vm vm,
                   wabi_map map,
                   wabi_map_entry entry,
                   wabi_word_t hash,
                   int hash_offset)
{
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ARRAY:
    return (wabi_map) wabi_map_array_assoc_rec(vm, (wabi_map_array) map, entry, hash, hash_offset);
  case WABI_TAG_MAP_HASH:
    return (wabi_map) wabi_map_hash_assoc_rec(vm, (wabi_map_hash) map, entry, hash, hash_offset);
  default:
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
}


wabi_map
wabi_map_assoc_raw(wabi_vm vm,
                   wabi_map map,
                   wabi_val key,
                   wabi_val value)
{
  wabi_word_t hash = wabi_hash_raw(key);
  wabi_map_entry entry = (wabi_map_entry) wabi_mem_allocate(vm, WABI_MAP_SIZE);
  if(vm->errno) return NULL;

  return wabi_map_assoc_rec(vm, map, entry, hash, WABI_MAP_INITIAL_OFFSET);
}


wabi_val
wabi_map_assoc(wabi_vm vm,
               wabi_val map,
               wabi_val key,
               wabi_val value)
{
  if(wabi_val_is_nil(map)) {
    map = wabi_map_empty(vm);
    if(vm->errno) return NULL;
  }
  if(wabi_val_is_map(map)) {
    return (wabi_val) wabi_map_assoc_raw(vm, (wabi_map) map, key, value);
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}


/**
 * GET Operation
 */

wabi_val
wabi_map_hash_get_rec(wabi_map_hash map,
                      wabi_val key,
                      wabi_word_t hash,
                      int hash_offset);


wabi_val
wabi_map_array_get_rec(wabi_map_array map,
                       wabi_val key,
                       wabi_word_t hash,
                       int hash_offset);


wabi_val
wabi_map_hash_get_rec(wabi_map_hash map,
                      wabi_val key,
                      wabi_word_t hash,
                      int hash_offset)
{
  wabi_val key0;
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_map table = WABI_MAP_HASH_TABLE(map);
  wabi_word_t index = WABI_MAP_HASH_INDEX(hash, hash_offset);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
    wabi_map child = table + offset;
    switch(wabi_val_tag((wabi_val) child)) {
    case WABI_TAG_MAP_ENTRY:
      key0 = WABI_MAP_ENTRY_KEY((wabi_map_entry) child);
      if(wabi_eq_raw(key, key0)) {
        return (wabi_val) WABI_MAP_ENTRY_VALUE((wabi_map_entry) child);
      }
      return NULL;
    case WABI_TAG_MAP_HASH:
      return wabi_map_hash_get_rec((wabi_map_hash) child, key, hash, hash_offset + 6);
    case WABI_TAG_MAP_ARRAY:
      return wabi_map_array_get_rec((wabi_map_array) child, key, hash, hash_offset + 6);
    }
  }
  return NULL;
}

wabi_val
wabi_map_array_get_rec(wabi_map_array map,
                       wabi_val key,
                       wabi_word_t hash,
                       int hash_offset)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  wabi_map_entry child = (wabi_map_entry) table;
  wabi_map_entry limit = (wabi_map_entry) table + size;

  // todo: consider that the table is ordered ;)
  while(child < limit) {
    wabi_val key0 = (wabi_val) WABI_MAP_ENTRY_KEY(child);
    if(wabi_eq_raw(key, key0)) {
      return (wabi_val) WABI_MAP_ENTRY_VALUE(child);
    }
    child++;
  }
  return NULL;
}


wabi_val
wabi_map_get_rec(wabi_map map,
                 wabi_val key,
                 wabi_word_t hash,
                 int hash_offset)
{
  // todo: convert in a single loop (or exploit TCO opt in GCC?)
  if(wabi_val_is_map_array((wabi_val) map)) {
    return wabi_map_array_get_rec((wabi_map_array) map, key, hash, hash_offset);
  }
  return wabi_map_hash_get_rec((wabi_map_hash) map, key, hash, hash_offset);
}


wabi_val
wabi_map_get_raw(wabi_map map,
                 wabi_val key)
{
  wabi_word_t hash = wabi_hash_raw(key);
  return wabi_map_get_rec(map, key, hash, WABI_MAP_INITIAL_OFFSET);
}


wabi_val
wabi_map_get(wabi_vm vm,
             wabi_val map,
             wabi_val key)
{
  if(wabi_val_is_nil(map)) {
    return map;
  }
  if(wabi_val_is_map(map)) {
    wabi_val res = wabi_map_get_raw((wabi_map) map, key);
    if(res) return res;
    return wabi_nil(vm);
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}


/**
 * EMPTY MAP
 */

wabi_val
wabi_map_empty(wabi_vm vm)
{
  wabi_map_array map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE);
  if(vm->errno) return NULL;

  map->size = 0UL;
  map->table = WABI_TAG_MAP_ARRAY;
  return (wabi_val) map;
}
