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


wabi_hamt_table
wabi_hamt_get_table(wabi_hamt_map map, wabi_hamt_index index)
{
  if(wabi_hamt_bit_get(map->bitmap, index)) {
    wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK);
    return table + wabi_hamt_table_offset(map->bitmap, index);
  }

  return NULL;
}


wabi_hamt_map
wabi_hamt_set_table(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index index, wabi_hamt_table elem)
{
  wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK);
  wabi_word_t size = WABI_POPCNT(map->bitmap);
  wabi_word_t pivot = wabi_hamt_table_offset(map->bitmap, index);
  wabi_hamt_table new_table = (wabi_hamt_table) wabi_mem_allocate(vm, WABI_HAMT_SIZE * (size + 1));
  if(vm->errno) return NULL;

  memcpy(new_table, table, WABI_WORD_SIZE * WABI_HAMT_SIZE * pivot);
  *(new_table + pivot) = *elem;
  memcpy(new_table + pivot + 1, table + pivot, WABI_WORD_SIZE * WABI_HAMT_SIZE * (size - pivot));

  wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP;
  new_map->bitmap = wabi_hamt_bit_set(map->bitmap, index);

  // printf("created map: %p of length: %i, adding element at %i\n", new_map, size + 1, index);
  return new_map;
}


wabi_hamt_entry
wabi_hamt_get_entry(wabi_hamt_map map,
                    wabi_hamt_index offset,
                    wabi_obj key,
                    wabi_word_t hash)
{
  wabi_word_t index;
  wabi_hamt_table table;

  do {
    index = wabi_hamt_index(hash, offset);
    table = wabi_hamt_get_table(map, index);
    if(table && wabi_hamt_is_map(table)) {
      map = (wabi_hamt_map) table;
      offset += 6;
      continue;
    }
    return (wabi_hamt_entry) table;
  } while(offset < 64); // todo use the constant
  return NULL;
}


wabi_hamt_map
wabi_hamt_set_entry(wabi_vm vm,
                    wabi_hamt_map map,
                    wabi_hamt_index offset,
                    wabi_obj key,
                    wabi_word_t hash,
                    wabi_hamt_entry entry
                    )
{
  wabi_hamt_index index = wabi_hamt_index(hash, offset);
  printf("HASH: %lx INDEX: %lx\n", hash, index);
  return wabi_hamt_set_table(vm, map, index, (wabi_hamt_table) entry);
}


wabi_hamt_map
wabi_hamt_set_raw(wabi_vm vm, wabi_hamt_map map, wabi_obj key, wabi_obj value)
{
  wabi_word_t hash = wabi_hash_raw(key);
  wabi_hamt_entry_t entry;
  entry.key = (wabi_word_t) key;
  entry.value = (wabi_word_t) value;

  return wabi_hamt_set_entry(vm, map, 0, key, hash, &entry);
}


wabi_obj
wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val)
{
  if(wabi_obj_is_nil(map)) {
    map = wabi_hamt_empty(vm);
    if(vm->errno) return NULL;
  }
  if(!wabi_obj_is_hamt_map(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  return (wabi_obj) wabi_hamt_set_raw(vm, (wabi_hamt_map) map, key, val);
}


wabi_obj
wabi_hamt_get_raw(wabi_hamt_map map, wabi_obj key)
{
  wabi_word_t hash = wabi_hash_raw(key);
  wabi_hamt_entry entry =  wabi_hamt_get_entry(map, 0, key, hash);

  return entry ? (wabi_obj) entry->value : NULL;
}


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key)
{
  if(!wabi_obj_is_hamt_map(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_obj res =  wabi_hamt_get_raw((wabi_hamt_map) map, key);
  if(res) return res;
  return wabi_nil(vm);
}


wabi_obj
wabi_hamt_empty(wabi_vm vm)
{
  wabi_hamt_map res = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  res->bitmap = 0;
  res->table = WABI_TAG_HAMT_MAP;
  return (wabi_obj) res;
}
