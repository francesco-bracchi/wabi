/**
 * Clojure is still faster.
 */

#define wabi_hamt_c

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_hash.h"
#include "wabi_atomic.h"
#include "wabi_hamt.h"
#include "wabi_eq.h"


wabi_hamt_table
wabi_hamt_table_get(wabi_hamt_map map, wabi_hamt_index index)
{
  wabi_word_t bitmap = MAP_BITMAP(map);
  wabi_hamt_table table = MAP_TABLE(map);
  if(BITMAP_FOUND(bitmap, index)) {
    wabi_word_t offset = BITMAP_OFFSET(bitmap, index);
    return table + offset;
  }
  return NULL;
}


wabi_hamt_map
wabi_hamt_table_add(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index index, wabi_hamt_table val)
{
  wabi_word_t bitmap = MAP_BITMAP(map);
  wabi_hamt_table table = MAP_TABLE(map);
  wabi_word_t size = (wabi_word_t) BITMAP_SIZE(bitmap);
  wabi_hamt_index offset = BITMAP_OFFSET(bitmap, index);
  wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, (2 + size) * WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  wabi_hamt_table new_table = (wabi_hamt_table) (new_map + 1);
  memcpy(new_table, table, WABI_WORD_SIZE * WABI_HAMT_SIZE * offset);
  *(new_table + offset) = (wabi_hamt_table_t) *val;
  memcpy(new_table + offset + 1, table + offset, WABI_WORD_SIZE * WABI_HAMT_SIZE * (size - offset));

  new_map->bitmap = bitmap | (1UL << index);
  new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP;
  return new_map;
}


wabi_hamt_map
wabi_hamt_table_update(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index index, wabi_hamt_table val)
{
  wabi_hamt_table table = MAP_TABLE(map);
  wabi_word_t size = (wabi_word_t) BITMAP_SIZE(map->bitmap);
  wabi_hamt_index offset = BITMAP_OFFSET(map->bitmap, index);
  wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, (1 + size) * WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  wabi_hamt_table new_table = (wabi_hamt_table) (new_map + 1);
  memcpy(new_table, table, size * WABI_HAMT_SIZE * WABI_WORD_SIZE);
  *(new_table + offset) = *val;

  new_map->bitmap = map->bitmap;
  new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP;

  return new_map;
}


wabi_hamt_map
wabi_hamt_table_remove(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index index)
{
  wabi_hamt_table table = MAP_TABLE(map);
  wabi_word_t bitmap = map->bitmap;
  wabi_word_t size = (wabi_word_t) BITMAP_SIZE(bitmap);
  wabi_hamt_index offset = BITMAP_OFFSET(map->bitmap, index);
  wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, size * WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  wabi_hamt_table new_table = (wabi_hamt_table) (new_map + 1);
  memcpy(new_table, table, WABI_WORD_SIZE * WABI_HAMT_SIZE * offset);
  memcpy(new_table + offset, table + offset + 1, WABI_WORD_SIZE * WABI_HAMT_SIZE * (size - offset - 1L));

  new_map->bitmap = map->bitmap ^ (1UL << index);
  new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP;
  return new_map;
}


wabi_hamt_entry
wabi_hamt_entry_get(wabi_hamt_map map, wabi_hamt_index h_pos, wabi_word_t hash)
{
  wabi_word_t index;
  wabi_hamt_table row;

  do {
    index = HASH_INDEX(hash, h_pos);
    row = wabi_hamt_table_get(map, index);
    if(! row) {
      return NULL;
    }
    if(wabi_obj_is_hamt_map((wabi_obj) row)) {
      map = (wabi_hamt_map) row;
      h_pos -= 6;
      continue;
    }
  } while(1);
  return (wabi_hamt_entry) row;
}


wabi_hamt_map
wabi_hamt_merge(wabi_vm vm,
                wabi_hamt_index h_pos,
                wabi_word_t hash, wabi_hamt_entry entry,
                wabi_word_t hash0, wabi_hamt_entry entry0)
{
  wabi_hamt_index index = HASH_INDEX(hash, h_pos);
  wabi_hamt_index index0 = HASH_INDEX(hash0, h_pos);

  if(index == index0) {
    wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, 2 * WABI_HAMT_SIZE);
    if(vm->errno) return NULL;

    wabi_hamt_table table = (wabi_hamt_table) (map + 1);

    wabi_hamt_map submap = wabi_hamt_merge(vm, h_pos - 6, hash, entry, hash0, entry0);
    if(vm->errno) return NULL;

    *table = (wabi_hamt_table_t) *submap;
    map->bitmap = (1UL << index) | (1UL << index0);
    map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP;
    return map;
  }
  wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, 3 * WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  wabi_hamt_table table = (wabi_hamt_table) (map + 1);

  if(index < index0) {
    *table = (wabi_hamt_table_t) *entry;
    *(table + 1) = (wabi_hamt_table_t) *entry0;

    map->bitmap = (1UL << index) | (1UL << index0);
    map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP;
    return map;
  }
  else {
    *table = (wabi_hamt_table_t) *entry0;
    *(table + 1) = (wabi_hamt_table_t) *entry;

    map->bitmap = (1UL << index) | (1UL << index0);
    map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP;
    return map;
  }
}


wabi_hamt_map
wabi_hamt_set_entry(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index h_pos, wabi_word_t hash, wabi_hamt_entry entry)
{
  wabi_hamt_index index = HASH_INDEX(hash, h_pos);
  wabi_hamt_table row = wabi_hamt_table_get(map, index);
  if(row) {
    if(wabi_obj_is_hamt_map((wabi_obj) row)) {
      wabi_hamt_map submap =  wabi_hamt_set_entry(vm, (wabi_hamt_map) row, h_pos - 6, hash, entry);
      return wabi_hamt_table_update(vm, map, index, (wabi_hamt_table) submap);
    }
    wabi_hamt_entry entry0 = (wabi_hamt_entry) row;

    wabi_word_t hash0 = wabi_hash_raw((wabi_obj) entry0->key);
    wabi_hamt_map submap = wabi_hamt_merge(vm, h_pos - 6, hash, entry, hash0, entry0);
    return wabi_hamt_table_update(vm, map, index, (wabi_hamt_table) submap);
  }
  return wabi_hamt_table_add(vm, map, index, (wabi_hamt_table) entry);
}


wabi_hamt_map
wabi_hamt_dissoc_entry(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index h_pos, wabi_word_t hash)
{
  wabi_hamt_index index = HASH_INDEX(hash, h_pos);
  wabi_hamt_table row = wabi_hamt_table_get(map, index);

  if(row) {
    if(wabi_obj_is_hamt_entry((wabi_obj) row)) {
      wabi_hamt_map res = wabi_hamt_table_remove(vm, map, index);
      return res;
    }
    wabi_hamt_map submap =  wabi_hamt_dissoc_entry(vm, (wabi_hamt_map) row, h_pos - 6, hash);
    return wabi_hamt_table_update(vm, map, index, (wabi_hamt_table) submap);
  }
  printf("NOT FOUND %i %p\n", row);
  return map;
}


wabi_obj
wabi_hamt_empty(wabi_vm vm) {
  wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  if(vm->errno) return NULL;
  map->bitmap = 0;
  map->table = WABI_TAG_HAMT_MAP;
  return (wabi_obj) map;
}


wabi_obj
wabi_hamt_assoc(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val)
{
  if(wabi_obj_is_nil(map)) {
    map = wabi_hamt_empty(vm);
    if(vm->errno) return NULL;
  }
  if(!wabi_obj_is_hamt_map(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_hamt_entry_t entry;
  entry.key = (wabi_word_t) key;
  entry.value = (wabi_word_t) val | WABI_TAG_HAMT_ENTRY;
  wabi_word_t hash = wabi_hash_raw(key);
  if(vm->errno) return NULL;
  return (wabi_obj)
    wabi_hamt_set_entry(vm, (wabi_hamt_map) map, (wabi_hamt_index) 50, hash, &entry);
}


int64_t
wabi_hamt_length_raw(wabi_hamt_map map)
{
  wabi_hamt_table table = MAP_TABLE(map);
  wabi_size_t size = WABI_POPCNT(MAP_BITMAP(map));
  int64_t total = 0;

  for(wabi_size_t j = 0; j < size; j++) {
    wabi_hamt_table row = table + j;
    if(wabi_obj_is_hamt_map((wabi_obj) row)) {
      total += wabi_hamt_length_raw((wabi_hamt_map) row);
    } else {
      total++;
    }
  }
  return total;
}


wabi_obj
wabi_hamt_length(wabi_vm vm, wabi_obj map) {
  if(wabi_obj_is_nil(map)) {
    return wabi_smallint(vm, 0);
  }
  if(!wabi_obj_is_hamt_map(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return wabi_smallint(vm, wabi_hamt_length_raw((wabi_hamt_map) map));
}


wabi_obj
wabi_hamt_get_raw(wabi_obj map, wabi_obj key)
{
  wabi_word_t hash = wabi_hash_raw(key);
  wabi_hamt_entry entry = wabi_hamt_entry_get((wabi_hamt_map) map, 50, hash);
  if(entry && wabi_eq_raw((wabi_obj) ENTRY_KEY(entry), key)) {
    return (wabi_obj) ENTRY_VALUE(entry);
  }
  return NULL;
}


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key)
{
  if(wabi_obj_is_nil(map)) {
    return map;
  }
  if(wabi_obj_is_hamt_map(map)) {
    wabi_obj res = wabi_hamt_get_raw(map, key);
    return res ? res : wabi_nil(vm);
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}


wabi_obj
wabi_hamt_dissoc(wabi_vm vm, wabi_obj map, wabi_obj key)
{
  if(wabi_obj_is_nil(map)) {
    return map;
  }
  if(wabi_obj_is_hamt_map(map)) {
    wabi_word_t hash = wabi_hash_raw(key);
    return (wabi_obj) wabi_hamt_dissoc_entry(vm, (wabi_hamt_map) map, (wabi_hamt_index) 50, hash);
  }
  vm->errno = WABI_ERROR_TYPE_MISMATCH;
  return NULL;
}
