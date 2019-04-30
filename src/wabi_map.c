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
  new_map->table = WABI_TAG_MAP_HASH;

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

// todo split in a couple of subroutines
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
    int cmp = wabi_cmp_raw(key, key0);
    if(cmp > 0) {
      row++;
      continue;
    } else if(cmp == 0) {
      // key found => replace
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
    break;
  }
  // if not found and the hash is exhausted and the array map is bigger than the limit
  if(hash_offset > 0 && size >= WABI_MAP_ARRAY_LIMIT) {
    wabi_map_hash hash_map = wabi_map_array_promote(vm, map, hash_offset);
    if(vm->errno) return NULL;
    return wabi_map_hash_assoc_rec(vm, hash_map, entry, hash, hash_offset);
  }
  // if not found and hash is exhausted, or the array map has more free entries
  wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 2));
  if(vm->errno) return NULL;

  wabi_map new_table = (wabi_map) (new_map + 1);
  wabi_word_t pos = row - table;

  memcpy(new_table, table, WABI_MAP_BYTE_SIZE * pos);
  *(new_table + pos) = (wabi_map_t) *entry;
  memcpy(new_table + pos + 1, table + pos, WABI_MAP_BYTE_SIZE * (size - pos));

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

  new_map->bitmap = bitmap | (1UL << index);
  new_map->table = ((wabi_word_t) new_table) | WABI_TAG_MAP_HASH;

  return (wabi_map) new_map;
}


static inline wabi_map
wabi_map_entry_assoc_rec(wabi_vm vm,
                         wabi_map_entry entry0,
                         wabi_map_entry entry,
                         wabi_word_t hash,
                         int hash_offset)
{
  int cmp = wabi_cmp_raw(WABI_MAP_ENTRY_KEY(entry0), WABI_MAP_ENTRY_KEY(entry));
  if(! cmp) return (wabi_map) entry;

  wabi_map_array map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * 3);
  if(vm->errno) return NULL;
  wabi_map table = (wabi_map) (map + 1);
  if(cmp > 0) {
    *table = (wabi_map_t) *entry0;
    *(table + 1) = (wabi_map_t) *entry;
  } else {
    *table = (wabi_map_t) *entry;
    *(table + 1) = (wabi_map_t) *entry0;
  }
  map->table = (wabi_word_t) table | WABI_TAG_MAP_ARRAY;
  map->size = 2UL;
  return (wabi_map) map;
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
  case WABI_TAG_MAP_ENTRY:
    return (wabi_map) wabi_map_entry_assoc_rec(vm, (wabi_map_entry) map, entry, hash, hash_offset);
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
  entry->key = (wabi_word_t) key;
  entry->value = (wabi_word_t) value | WABI_TAG_MAP_ENTRY;
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
 * DISSOC operation
 */

static wabi_map
wabi_map_dissoc_rec(wabi_vm vm, wabi_map map, wabi_val key, wabi_word_t hash, int hash_offset);


static inline wabi_map
wabi_map_entry_dissoc_rec(wabi_vm vm,
                      wabi_map_entry entry,
                      wabi_val key,
                      wabi_word_t hash,
                      int hash_offset)
{
  return wabi_eq_raw(WABI_MAP_ENTRY_KEY(entry), key) ? NULL : (wabi_map) entry;
}

static inline wabi_map
wabi_map_array_dissoc_rec(wabi_vm vm,
                      wabi_map_array map,
                      wabi_val key,
                      wabi_word_t hash,
                      int hash_offset)
{
  wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  wabi_word_t size = WABI_MAP_ARRAY_SIZE(map);
  wabi_map row = table;
  wabi_map limit = table + size;
  while(row < limit) {
    wabi_val key0 = WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    int cmp = wabi_cmp_raw(key, key0);
    if(cmp > 0) {
      row++;
      continue;
    } else if(cmp == 0) {
      // key found
      wabi_word_t offset = row - table;
      wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * size);
      if(vm->errno) return NULL;

      wabi_map new_table = (wabi_map) (new_map + 1);
      memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
      memcpy(new_table + offset, table + offset + 1 , WABI_MAP_BYTE_SIZE * (size - offset - 1));

      new_map->size = size - 1;
      new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
      return (wabi_map) new_map;
    }
    else {
      // not found;
      return (wabi_map) map;
    }
  }
  // not found;
  return (wabi_map) map;
}


static inline void
wabi_map_insert_sort(wabi_map_entry table,
                     int size)
{
  int i, j;
  wabi_map_entry_t x;

  while(i < size) {
    x = *(table + i);
    j = i - 1;
    while(j >= 0 && wabi_cmp_raw(table + j, &x) > 0) {
      *(table + j + 1) = *(table + j);
      j--;
    }
    *(table + j + 1) = x;
    i++;
  }
}


static inline wabi_map
wabi_map_hash_demote(wabi_vm vm,
                     wabi_map_hash map)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  wabi_map limit = (wabi_map) (table + size);
  wabi_map row = table;
  wabi_word_t len = 0;
  while(row < limit) {
    switch(wabi_val_tag((wabi_val) row)) {
    case WABI_TAG_MAP_ENTRY:
      len++;
      if(len > WABI_MAP_ARRAY_LIMIT) return (wabi_map) map;
      break;
    case WABI_TAG_MAP_ARRAY:
      len+= WABI_MAP_ARRAY_SIZE((wabi_map_array) row);
      if(len > WABI_MAP_ARRAY_LIMIT) return (wabi_map) map;
      break;
    default:
      return (wabi_map) map;
    }
    row++;
  }
  wabi_map_array new_map = (wabi_map_array) wabi_mem_allocate(vm, WABI_MAP_SIZE * (1 + len));
  if(vm->errno) return NULL;
  wabi_map new_table = (wabi_map) (new_map + 1);

  wabi_map_iter_t iter;
  wabi_map_entry entry;
  row = new_table;
  wabi_map_iterator_init(&iter, (wabi_map) map);
  while((entry = wabi_map_iterator_current(&iter))) {
    *row = (wabi_map_t) *entry;
    wabi_map_iterator_next(&iter);
    row++;
  }
  wabi_map_insert_sort(new_table, size);

  new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_ARRAY;
  new_map->size = len;
  return (wabi_map) new_map;
}

static inline wabi_map
wabi_map_hash_dissoc_rec(wabi_vm vm,
                         wabi_map_hash map,
                         wabi_val key,
                         wabi_word_t hash,
                         int hash_offset)
{
  wabi_word_t bitmap = WABI_MAP_HASH_BITMAP(map);
  wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap);
  wabi_word_t index = WABI_MAP_HASH_INDEX(hash, hash_offset);
  wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
  wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    wabi_map_hash new_map;
    wabi_map row = table + offset;
    wabi_map sub_map = wabi_map_dissoc_rec(vm, row, key, hash, hash_offset - 6);
    if(!sub_map) {
      new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * size);
      if(vm->errno) return NULL;
      wabi_map new_table = (wabi_map) (new_map + 1);

      memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
      memcpy(new_table + offset, table + offset + 1L, (size - offset - 1L) * WABI_MAP_BYTE_SIZE);
      new_map->bitmap = bitmap ^ (1UL << index);
      new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_HASH;
    }
    else {
      new_map = (wabi_map_hash) wabi_mem_allocate(vm, WABI_MAP_SIZE * (size + 1));
      if(vm->errno) return NULL;
      wabi_map new_table = (wabi_map) (new_map + 1);
      memcpy(new_table, table, size * WABI_MAP_BYTE_SIZE);
      *(new_table + offset) = (wabi_map_t) *sub_map;
      new_map->bitmap = bitmap;
      new_map->table = (wabi_word_t) new_table | WABI_TAG_MAP_HASH;
    }
    if(size - 1 <= WABI_MAP_ARRAY_LIMIT) {
      return wabi_map_hash_demote(vm, (wabi_map_hash) new_map);
    }
    return (wabi_map) new_map;
  }
  return (wabi_map) map;
}


static wabi_map
wabi_map_dissoc_rec(wabi_vm vm, wabi_map map, wabi_val key, wabi_word_t hash, int hash_offset)
{
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ENTRY:
    return wabi_map_entry_dissoc_rec(vm, (wabi_map_entry) map, key, hash, hash_offset);
  case WABI_TAG_MAP_ARRAY:
    return wabi_map_array_dissoc_rec(vm, (wabi_map_array) map, key, hash, hash_offset);
  case WABI_TAG_MAP_HASH:
    return wabi_map_hash_dissoc_rec(vm, (wabi_map_hash) map, key, hash, hash_offset);
  default:
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
}


wabi_map
wabi_map_dissoc_raw(wabi_vm vm,
                    wabi_map map,
                    wabi_val key)
{
  wabi_word_t hash = wabi_hash_raw(key);
  return wabi_map_dissoc_rec(vm, map, key, hash, WABI_MAP_INITIAL_OFFSET);
}



wabi_val
wabi_map_dissoc(wabi_vm vm,
                wabi_val map,
                wabi_val key)
{
  if(wabi_val_is_nil(map)) {
    return wabi_map_empty(vm);
  }
  if(wabi_val_is_map(map)) {
    return (wabi_val) wabi_map_dissoc_raw(vm, (wabi_map) map, key);
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
      return wabi_map_hash_get_rec((wabi_map_hash) child, key, hash, hash_offset - 6);
    case WABI_TAG_MAP_ARRAY:
      return wabi_map_array_get_rec((wabi_map_array) child, key, hash, hash_offset - 6);
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

  while(child < limit) {
    wabi_val key0 = (wabi_val) WABI_MAP_ENTRY_KEY(child);
    int cmp = wabi_cmp_raw(key, key0);
    if(cmp > 0) {
      child++;
      continue;
    } else if(cmp == 0) {
      return (wabi_val) WABI_MAP_ENTRY_VALUE(child);
    } else {
      return NULL;
    }
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
 * Iterator
 */

static inline void
wabi_map_iterator_grow(wabi_map_iter iter)
{
  wabi_map_iter_frame frame;
  wabi_map map, table;
  wabi_word_t bitmap, size;
  int pos;

  if(iter->top < 0) return;

  do {
    frame = iter->stack + iter->top;
    map = frame->map;
    switch(wabi_val_tag((wabi_val) map)) {
    case WABI_TAG_MAP_ENTRY:
      return;
    case WABI_TAG_MAP_ARRAY:
      size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
      if (frame->pos >= size) {
        iter->top = -1;
        return;
      }
      pos = frame->pos;
      iter->top++;
      frame++;
      frame->map = WABI_MAP_ARRAY_TABLE((wabi_map_array) map) + pos;
      frame->pos = 0;
      break;
    case WABI_TAG_MAP_HASH:  // default:
      bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
      size = WABI_MAP_BITMAP_COUNT(bitmap);
      if (frame->pos >= size) {
        iter->top = -1;
        return;
      }
      pos = frame->pos;
      table = WABI_MAP_HASH_TABLE((wabi_map_hash) map) + pos;
      iter->top++;
      frame++;
      frame->map = table;
      frame->pos = 0;
      break;
    default:
      return;
    }
  }
  while(iter->top < WABI_MAP_ITER_STACK_SIZE);
}


static inline int
wabi_map_iterator_frame_full(wabi_map_iter_frame frame) {
  wabi_word_t size, bitmap;
  wabi_map map = frame->map;
  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ENTRY:
    return 1;
  case WABI_TAG_MAP_ARRAY:
    size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
    return frame->pos + 1 >= size;
  case WABI_TAG_MAP_HASH:
    bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
    size = WABI_MAP_BITMAP_COUNT(bitmap);
    return frame->pos + 1 >= size;
  }
  return 0;
}


static void
wabi_map_iterator_shrink(wabi_map_iter iter)
{
  while(wabi_map_iterator_frame_full(iter->stack + iter->top)) {
    iter->top--;
    if(iter->top < 0) return;
  }
  (iter->stack + iter->top)->pos++;
}


void
wabi_map_iterator_init(wabi_map_iter iter,
                       wabi_map map)
{
  wabi_map_iter_frame frame = iter->stack;
  iter->top = 0;
  frame->map = map;
  frame->pos = 0;
  wabi_map_iterator_grow(iter);
}


wabi_map_entry
wabi_map_iterator_current(wabi_map_iter iter) {
  if(iter->top < 0) return NULL;
  wabi_map_iter_frame frame = iter->stack + iter->top;
  return (wabi_map_entry) frame->map;
}


void
wabi_map_iterator_next(wabi_map_iter iter) {
  wabi_map_iterator_shrink(iter);
  wabi_map_iterator_grow(iter);
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


/**
 * Length
 */

wabi_word_t
wabi_map_length_raw(wabi_map map) {
  wabi_map table, limit;
  wabi_word_t size, bitmap;

  switch(wabi_val_tag((wabi_val) map)) {
  case WABI_TAG_MAP_ARRAY:
    return WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
  case WABI_TAG_MAP_HASH:
    table = WABI_MAP_HASH_TABLE((wabi_map_hash) map);
    bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
    limit = table + WABI_MAP_BITMAP_COUNT(bitmap);
    size = 0;
    while(table < limit) {
      size += wabi_map_length_raw(table);
      table++;
    }
    return size;
  case WABI_TAG_MAP_ENTRY:
    return 1;
  }
  return 0;
}


wabi_val
wabi_map_length(wabi_vm vm, wabi_val map)
{
  if(!wabi_val_is_map(map)) {
    vm->errno = WABI_ERROR_TYPE_MISMATCH;
    return NULL;
  }
  wabi_word_t len = wabi_map_length_raw((wabi_map) map);
  return wabi_smallint(vm, len);
}





/* /\*** TEST ***\/ */

/* void do_indent(int x) { */
/*   for(int j = 0; j < x; j++) putchar(' '); */
/* } */

/* void wabi_struct(wabi_map map, int indent, int hash_offset); */

/* void wabi_struct_array(wabi_map map, int indent, int hash_offset) { */
/*   wabi_map table = (wabi_map) WABI_MAP_ARRAY_TABLE((wabi_map_array) map); */
/*   wabi_word_t size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map); */

/*   do_indent(indent); */
/*   printf("ARRAY [\n"); */
/*   for(wabi_word_t index = 0; index < size; index++) { */
/*     do_indent(indent); */
/*     printf("%lu:\n", index); */
/*     wabi_struct(table + index, indent + 2, hash_offset - 6); */
/*     printf("\n"); */
/*   } */
/*   do_indent(indent); */
/*   printf("]"); */
/* } */


/* void wabi_struct_hash(wabi_map map, int indent, int hash_offset) { */
/*   wabi_word_t bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map); */
/*   wabi_word_t size = WABI_MAP_BITMAP_COUNT(bitmap); */
/*   wabi_map table = (wabi_map) WABI_MAP_HASH_TABLE((wabi_map_hash) map); */
/*   wabi_word_t offset = 0; */
/*   do_indent(indent); */
/*   printf("HASH %lu {\n", hash_offset); */
/*   for(wabi_word_t index = 0; index < 64; index++) { */
/*     if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) { */
/*       wabi_word_t offset = WABI_MAP_BITMAP_OFFSET(bitmap, index); */
/*       wabi_map submap = table + offset; */
/*       do_indent(indent); */
/*       printf("%lu:\n", index); */
/*       wabi_struct(submap, indent + 2, hash_offset - 6); */
/*       printf("\n"); */
/*       offset++; */
/*     } */
/*   } */

/*   do_indent(indent); */
/*   printf("}"); */
/* } */

/* void wabi_struct(wabi_map map, int indent, int hash_offset) */
/* { */
/*   switch(wabi_val_tag((wabi_val) map)) { */
/*   case WABI_TAG_MAP_ENTRY: */
/*     do_indent(indent); */
/*     wabi_pr(map); */
/*     break; */
/*   case WABI_TAG_MAP_ARRAY: */
/*     wabi_struct_array(map, indent, hash_offset); */
/*     break; */
/*   case WABI_TAG_MAP_HASH: */
/*     wabi_struct_hash(map, indent, hash_offset); */
/*     break; */
/*   default: */
/*     printf("unknown: %lx", wabi_val_tag((wabi_val) map)); */
/*   } */
/* } */

/* void wabi_str(wabi_map map) */
/* { */
/*   wabi_struct(map, 0, WABI_MAP_INITIAL_OFFSET); */
/* } */
