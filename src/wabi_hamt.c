#define wabi_hamt_c

#include <stddef.h>

#include "wabi_object.h"
#include "wabi_vm.h"
#include "wabi_mem.h"
#include "wabi_hash.h"
#include "wabi_atomic.h"
#include "wabi_hamt.h"


#define WABI_POPCNT(v) __builtin_popcountl(v)


wabi_obj
wabi_hamt_empty(wabi_vm vm)
{
  wabi_hamt_map res = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  res->bitmap = WABI_TAG_HAMT;
  res->table = (wabi_word_t) NULL;
  return (wabi_obj) res;
}


wabi_hamt_entry
wabi_hamt_get_index(wabi_hamt_map map, wabi_word_t index)
{
  wabi_word_t bitmap = map->bitmap;
  wabi_hamt_entry_t *table = (wabi_hamt_entry_t *) (map->table & WABI_VALUE_MASK);
  wabi_word_t is_found = (bitmap >> index) & 1;

  if(is_found) {
    short unsigned int offset = WABI_POPCNT(bitmap << (64 - index));
    return table + offset;
  }

  return NULL;
}


wabi_hamt_map
wabi_hamt_set_index(wabi_vm vm, wabi_hamt_map map, wabi_word_t index, wabi_hamt_entry value)
{
  wabi_word_t bitmap = map->bitmap;
  wabi_word_t *table = (wabi_word_t*) (map->table & WABI_VALUE_MASK);
  wabi_word_t offset = WABI_POPCNT(bitmap << (64 - index));
  wabi_word_t size = WABI_POPCNT(bitmap);

  wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  new_map->bitmap = bitmap & (1 << index);

  wabi_word_t *new_table = (wabi_word_t *) wabi_mem_allocate(vm, size + 1);
  if(vm->errno) return NULL;
  new_map->table = (wabi_word_t) table | WABI_TAG_HAMT;

  for (short unsigned int j = 0; j <= size; j++) {
    if (j < offset) {
      *(new_table + j) = *(table + j);
    } else if (j == offset) {
      *(new_table + j) = (wabi_word_t) value;
    } else {
      *(new_table + j) = *(table + j - 1);
    }
  }

  return new_map;
}


wabi_obj
wabi_hamt_get_raw(wabi_hamt_map map, wabi_word_t hash)
{
  wabi_hamt_entry entry;
  int offset = 50;
  while (offset > 0) {
    entry = wabi_hamt_get_index(map, hash >> offset && 0x3F);
    if(! entry) {
      break;
    }
    if(wabi_obj_is_hamt((wabi_obj) entry)) {
      offset -= 6;
      continue;
    }
    return (wabi_obj) (entry->pair.value | WABI_VALUE_MASK);
  }
  return NULL;
}


wabi_obj
wabi_hamt_set_raw(wabi_vm vm, wabi_hamt_map map, wabi_word_t hash, short unsigned int offset, wabi_hamt_pair pair)
{
  wabi_word_t index = hash >> offset && 0x3F;
  wabi_hamt_entry entry = wabi_hamt_get_index(map, index);
  if(wabi_obj_is_hamt((wabi_obj) entry)) {
    wabi_obj new_map = wabi_hamt_set_raw(vm, (wabi_hamt_map) entry, hash, offset - 6, pair);
    return (wabi_obj) wabi_hamt_set_index(vm, map, index, (wabi_hamt_entry) new_map);
  }
  return (wabi_obj) wabi_hamt_set_index(vm, map, index, (wabi_hamt_entry) pair);
}


wabi_obj
wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key)
{
  if(!wabi_obj_is_hamt(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }

  wabi_word_t hash = wabi_hash_raw(key);
  wabi_obj res = wabi_hamt_get_raw((wabi_hamt_map) map, hash);

  if(res) return res;
  return wabi_nil(vm);
}


wabi_obj
wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val)
{
  if(wabi_obj_is_nil(map)) {
    map = wabi_hamt_empty(vm);
    if(vm->errno)
      return NULL;
  }
  else if(!wabi_obj_is_hamt(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }

  wabi_word_t hash = wabi_hash_raw(key);
  wabi_hamt_pair pair = (wabi_hamt_pair) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  pair->value = (wabi_word_t) WABI_VALUE_MASK;
  pair->key =  (wabi_word_t) key;
  return wabi_hamt_set_raw(vm, (wabi_hamt_map) map, hash, 50, pair);
}
