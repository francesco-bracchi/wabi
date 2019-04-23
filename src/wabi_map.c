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
#include "wabi_eq.h"
#include "wabi_map.h"

wabi_map_hash
wabi_map_hash_assoc(wabi_vm vm,
                    wabi_map_hash map,
                    wabi_map_entry entry,
                    wabi_word_t hash,
                    int hash_offset);


wabi_map_table
wabi_map_array_assoc(wabi_vm vm,
                     wabi_map_array map,
                     wabi_map_entry entry,
                     wabi_word_t hash,
                     int hash_offset);


wabi_map_table
wabi_map_table_assoc(wabi_vm vm,
                     wabi_map_table map,
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

  wabi_map_table table = (wabi_map_table) WABI_MAP_ARRAY_TABLE(map);
  wabi_map_table limit = table + WABI_MAP_ARRAY_SIZE(map);
  wabi_map_table row = table;
  while(row < limit) {
    wabi_val key = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    wabi_word_t hash = wabi_hash_raw(key);
    new_map = wabi_map_hash_assoc(vm, new_map, (wabi_map_entry) row, hash, hash_offset);
    if(vm->errno) return NULL;
    row++;
  }
  return new_map;
}


wabi_map_table
wabi_map_array_assoc(wabi_vm vm,
                     wabi_map_array map,
                     wabi_map_entry entry,
                     wabi_word_t hash,
                     int hash_offset)
{
  wabi_val key = (wabi_val) WABI_MAP_ENTRY_KEY(entry);
  wabi_map_table table = (wabi_map_table) WABI_MAP_ARRAY_TABLE(map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  wabi_map_table row = table;
  wabi_map_table limit = table + size;

  // key lookup
  while(row < limit) {
    wabi_val key0 = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    if(wabi_eq_raw(key, key0)) {
      // if found replace the entry
      wabi_word_t pos = row - table;
      wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 1));
      if(vm->errno) return NULL;

      wabi_map_table new_table = (wabi_map_table) (new_map + 1);
      memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
      *(new_table + pos) = (wabi_map_table_t) *entry;

      new_map->size = size;
      new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
      return (wabi_map_table) new_map;
    }
    row++;
  }
  // if not found and the hash is not completely exahausted
  if(hash_offset > 0 && size >= WABI_MAP_ARRAY_LIMIT) {
    wabi_map_hash hash_map = wabi_map_array_promote(vm, map, hash_offset);
    if(vm->errno) return NULL;
    return (wabi_map_table) wabi_map_hash_assoc(vm, hash_map, entry, hash, hash_offset);
  }
  // if not found and hash is exhausted
  wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 2));
  if(vm->errno) return NULL;

  wabi_map_table new_table = (wabi_map_table) (new_map + 1);
  memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
  *(new_table + size) = (wabi_map_table_t) *entry;

  new_map->size = size + 1;
  new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
  return (wabi_map_table) new_map;
}


wabi_map_hash
wabi_map_hash_assoc(wabi_vm vm,
                    wabi_map_hash map,
                    wabi_map_entry entry,
                    wabi_word_t hash,
                    int hash_offset)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_word_t index = WABI_MAP_HASH_INDEX(hash, hash_offset);
  wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
  wabi_map_table table = (wabi_map_table) WABI_MAP_HASH_TABLE(map);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    wabi_map_table row = table + offset;

    wabi_map_hash new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 1));
    if(vm->errno) return NULL;

    wabi_map_table sub_map = wabi_map_table_assoc(vm, row, entry, hash, hash_offset - 6);
    if(vm->errno) return NULL;

    wabi_map_table new_table = (wabi_map_table) (new_map + 1);
    memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
    *(new_table + offset) = *sub_map;
    new_map->table = ((wabi_word_t) new_table) | WABI_TAG_MAP_HASH;
    new_map->bitmap = bitmap;
    return new_map;
  }
  wabi_map_hash new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 2));
  if(vm->errno) return NULL;
  wabi_map_table new_table = (wabi_map_table) (new_map + 1);

  memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
  *(new_table + offset) = (wabi_map_table_t) *entry;
  memcpy(new_table + offset + 1, table + offset, WABI_MAP_BYTE_SIZE * (size - offset));

  new_map->bitmap = bitmap & (1UL << index);
  new_map->table = ((wabi_word_t) new_table) | WABI_TAG_MAP_HASH;
  return new_map;
}


wabi_val
wabi_map_empty(wabi_vm vm)
{
  wabi_map_array map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE);
  if(vm->errno) return NULL;

  map->size = 0UL;
  map->table = WABI_TAG_MAP_ARRAY;
  return (wabi_val) map;
}


wabi_map_table
wabi_map_table_assoc(wabi_vm vm,
                     wabi_map_table map,
                     wabi_map_entry entry,
                     wabi_word_t hash,
                     int hash_offset)
{
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ARRAY:
    return (wabi_map_table) wabi_map_array_assoc(vm, (wabi_map_array) map, entry, hash, hash_offset);
  case WABI_TAG_MAP_HASH:
    return (wabi_map_table) wabi_map_hash_assoc(vm, (wabi_map_hash) map, entry, hash, hash_offset);
  default:
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
}


wabi_val
wabi_map_assoc(wabi_vm vm,
               wabi_val map,
               wabi_val key,
               wabi_val value)
{
  wabi_word_t hash = wabi_hash_raw(key);
  wabi_map_entry entry = (wabi_map_entry) wabi_mem_allocate(vm, WABI_MAP_SIZE);
  if(vm->errno) return NULL;

  if(wabi_val_is_nil(map)) {
    map = wabi_map_empty(vm);
    if(vm->errno) return NULL;
  }
  return (wabi_val) wabi_map_table_assoc(vm, (wabi_map_table) map, entry, hash, WABI_MAP_INITIAL_OFFSET);
}
