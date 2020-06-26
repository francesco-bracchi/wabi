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

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_hash.h"
#include "wabi_cmp.h"
#include "wabi_pair.h"
#include "wabi_constant.h"
#include "wabi_map.h"
#include "wabi_store.h"
#include "wabi_builtin.h"
#include "wabi_number.h"

#define WABI_MAP_HALLOC 80

/**
 * Assoc operation
 */
wabi_map
wabi_map_hash_assoc_rec(wabi_vm vm,
                        wabi_map_hash map,
                        wabi_map_entry entry,
                        wabi_word hash,
                        wabi_word hash_offset);


wabi_map
wabi_map_array_assoc_rec(wabi_vm vm,
                         wabi_map_array map,
                         wabi_map_entry entry,
                         wabi_word hash,
                         wabi_word hash_offset);


wabi_map
wabi_map_assoc_rec(wabi_vm vm,
                   wabi_map map,
                   wabi_map_entry entry,
                   wabi_word hash,
                   wabi_word hash_offset);


wabi_map_hash
wabi_map_array_promote(wabi_vm vm,
                       wabi_map_array map,
                       wabi_word hash_offset)
{
  wabi_map_hash new_map;
  wabi_map table, limit, row;
  wabi_val key;
  wabi_word hash;

  new_map = (wabi_map_hash) wabi_vm_alloc(vm, WABI_MAP_SIZE);
  if(new_map) {
    new_map->bitmap = 0UL;
    new_map->table = 0UL;
    WABI_SET_TAG(new_map, wabi_tag_map_hash);

    table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
    limit = table + WABI_MAP_ARRAY_SIZE(map);
    row = table;
    while(row < limit) {
      key = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
      hash = wabi_hash(key);
      new_map = (wabi_map_hash) wabi_map_hash_assoc_rec(vm, new_map, (wabi_map_entry) row, hash, hash_offset);
      if(!new_map) return NULL;
      row++;
    }
  }
  return new_map;
}

// todo split in a couple of subroutines
wabi_map
wabi_map_array_assoc_rec(wabi_vm vm,
                         wabi_map_array map,
                         wabi_map_entry entry,
                         wabi_word hash,
                         wabi_word hash_offset)
{
  wabi_val key, key0;
  wabi_map table, row, limit, new_table;
  wabi_word size, pos;
  wabi_map_array new_map;
  wabi_map_hash hash_map;
  int cmp;

  key = (wabi_val) WABI_MAP_ENTRY_KEY(entry);
  table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  size = WABI_MAP_ARRAY_SIZE(map);
  row = table;
  limit = table + size;

  while(row < limit) {
    key0 = (wabi_val) WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    cmp = wabi_cmp(key, key0);
    if(cmp < 0) {
      row++;
      continue;
    } else if(cmp == 0) {
      // key found => replace
      pos = row - table;
      new_map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE * (size + 1));
      if(new_map) {
        new_table = (wabi_map) (new_map + 1);
        memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
        *(new_table + pos) = (wabi_map_t) *entry;

        new_map->size = size;
        new_map->table = (wabi_word) new_table;
        WABI_SET_TAG(new_map, wabi_tag_map_array);
      }
      return (wabi_map) new_map;
    }
    break;
  }
  // if not found and the hash is exhausted and the array map is bigger than the limit
  if(hash_offset > 0 && size >= WABI_MAP_ARRAY_LIMIT) {
    hash_map = wabi_map_array_promote(vm, map, hash_offset);
    if(!hash_map) return NULL;
    return wabi_map_hash_assoc_rec(vm, hash_map, entry, hash, hash_offset);
  }
  // if not found and hash is exhausted, or the array map has more free entries
  new_map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE * (size + 2));
  if(new_map) {
    new_table = (wabi_map) (new_map + 1);
    pos = row - table;

    memcpy(new_table, table, WABI_MAP_BYTE_SIZE * pos);
    *(new_table + pos) = (wabi_map_t) *entry;
    memcpy(new_table + pos + 1, table + pos, WABI_MAP_BYTE_SIZE * (size - pos));

    new_map->size = size + 1;
    new_map->table = (wabi_word) new_table;
    WABI_SET_TAG(new_map, wabi_tag_map_array);
  }
  return (wabi_map) new_map;
}


wabi_map
wabi_map_hash_assoc_rec(wabi_vm vm,
                        wabi_map_hash map,
                        wabi_map_entry entry,
                        wabi_word hash,
                        wabi_word hash_offset)
{
  wabi_word bitmap, size, index, offset;
  wabi_map table, row, sub_map, new_table;
  wabi_map_hash new_map;

  bitmap = WABI_MAP_HASH_BITMAP(map);
  size = WABI_MAP_BITMAP_COUNT(bitmap);
  index = WABI_MAP_HASH_INDEX(hash, hash_offset);
  offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
  table = (wabi_map) WABI_MAP_HASH_TABLE(map);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    row = table + offset;
    new_map = (wabi_map_hash) wabi_vm_alloc(vm, WABI_MAP_SIZE * (size + 1));
    if(new_map) {
      sub_map = wabi_map_assoc_rec(vm, row, entry, hash, hash_offset - 6);
      if(! sub_map) return NULL;

      new_table = (wabi_map) (new_map + 1);
      memcpy(new_table, table, WABI_MAP_BYTE_SIZE * size);
      *(new_table + offset) = *sub_map;
      new_map->table = (wabi_word) new_table;
      new_map->bitmap = bitmap;
      WABI_SET_TAG(new_map, wabi_tag_map_hash);
    }
    return (wabi_map) new_map;
  }
  // offset: %lu index: %lu\n", offset, index);
  new_map = (wabi_map_hash) wabi_vm_alloc(vm, WABI_MAP_SIZE * (size + 2));
  if(new_map) {
    new_table = (wabi_map) (new_map + 1);
    memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
    *(new_table + offset) = (wabi_map_t) *entry;
    memcpy(new_table + offset + 1, table + offset, WABI_MAP_BYTE_SIZE * (size - offset));

    new_map->bitmap = bitmap | (1UL << index);
    new_map->table = (wabi_word) new_table;
    WABI_SET_TAG(new_map, wabi_tag_map_hash);
  }
  return (wabi_map) new_map;
}


wabi_map
wabi_map_entry_assoc_rec(wabi_vm vm,
                         wabi_map_entry entry0,
                         wabi_map_entry entry,
                         wabi_word hash,
                         wabi_word hash_offset)
{
  int cmp;
  wabi_map_array map;
  wabi_map table;

  cmp = wabi_cmp(WABI_MAP_ENTRY_KEY(entry0), WABI_MAP_ENTRY_KEY(entry));
  if(! cmp) return (wabi_map) entry;
  map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE * 3);
  if(map) {
    table = (wabi_map) (map + 1);
    if(cmp > 0) {
      *table = (wabi_map_t) *entry0;
      *(table + 1) = (wabi_map_t) *entry;
    } else {
      *table = (wabi_map_t) *entry;
      *(table + 1) = (wabi_map_t) *entry0;
    }
    map->table = (wabi_word) table;
    map->size = 2UL;
    WABI_SET_TAG(map, wabi_tag_map_array);
  }
  return (wabi_map) map;
}


wabi_map
wabi_map_assoc_rec(wabi_vm vm,
                   wabi_map map,
                   wabi_map_entry entry,
                   wabi_word hash,
                   wabi_word hash_offset)
{
  wabi_word tag;
  tag = WABI_TAG((wabi_val) map);

  if(tag == wabi_tag_map_hash) {
    return (wabi_map) wabi_map_hash_assoc_rec(vm, (wabi_map_hash) map, entry, hash, hash_offset);
  }
  if(tag == wabi_tag_map_array) {
    return (wabi_map) wabi_map_array_assoc_rec(vm, (wabi_map_array) map, entry, hash, hash_offset);
  }
  if(tag == wabi_tag_map_entry) {
    return (wabi_map) wabi_map_entry_assoc_rec(vm, (wabi_map_entry) map, entry, hash, hash_offset);
  }

  return NULL;
}


wabi_map
wabi_map_assoc(wabi_vm vm,
               wabi_map map,
               wabi_val key,
               wabi_val value)
{
  wabi_word hash;
  wabi_map_entry entry;

  hash = wabi_hash(key);
  entry = (wabi_map_entry) wabi_vm_alloc(vm, WABI_MAP_SIZE);
  if(entry) {
    entry->key = (wabi_word) key;
    entry->value = (wabi_word) value;
    WABI_SET_TAG(entry, wabi_tag_map_entry);
    return wabi_map_assoc_rec(vm, map, entry, hash, WABI_MAP_INITIAL_OFFSET);
  }
  return NULL;
}


/**
 * DISSOC operation
 */

 wabi_map
wabi_map_dissoc_rec(wabi_vm vm,
                    wabi_map map,
                    wabi_val key,
                    wabi_word hash,
                    wabi_word hash_offset);


wabi_map
wabi_map_entry_dissoc_rec(wabi_vm vm,
                      wabi_map_entry entry,
                      wabi_val key,
                      wabi_word hash,
                      wabi_word hash_offset)
{
  return !wabi_cmp(WABI_MAP_ENTRY_KEY(entry), key) ? NULL : (wabi_map) entry;
}

wabi_map
wabi_map_array_dissoc_rec(wabi_vm vm,
                      wabi_map_array map,
                      wabi_val key,
                      wabi_word hash,
                      wabi_word hash_offset)
{
  wabi_map table, row, limit, new_table;
  wabi_word size, offset;
  wabi_map_array new_map;
  wabi_val key0;
  int cmp;

  table = (wabi_map) WABI_MAP_ARRAY_TABLE(map);
  size = WABI_MAP_ARRAY_SIZE(map);
  row = table;
  limit = table + size;
  while(row < limit) {
    key0 = WABI_MAP_ENTRY_KEY((wabi_map_entry) row);
    cmp = wabi_cmp(key, key0);
    if(cmp < 0) {
      row++;
      continue;
    } else if(cmp == 0) {
      // key found
      offset = row - table;
      new_map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE * size);
      if(new_map) {
        new_table = (wabi_map) (new_map + 1);
        memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
        memcpy(new_table + offset, table + offset + 1 , WABI_MAP_BYTE_SIZE * (size - offset - 1));

        new_map->size = size - 1;
        new_map->table = (wabi_word) new_table;
        WABI_SET_TAG(new_map, wabi_tag_map_array);
      }
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


void
wabi_map_insert_sort(wabi_map_entry table,
                     int size)
{
  int i, j;
  wabi_map_entry_t tmp;
  wabi_val k, k0;

  i = 1;
  while(i <= size) {
    tmp = *(table + i);
    j = i - 1;
    k0 = WABI_MAP_ENTRY_KEY(table + i);
    k = WABI_MAP_ENTRY_KEY(table + j);
    while(j >= 0 && wabi_cmp(k, k0) < 0) {
      *(table + j + 1) = *(table + j);
      j--;
      k = WABI_MAP_ENTRY_KEY(table + j);
    }
    *(table + j + 1) = tmp;
    i++;
  }
}


wabi_map
wabi_map_hash_demote(wabi_vm vm,
                     wabi_map_hash map)
{
  wabi_word bitmap, size, tag, len;
  wabi_map table, limit, row, new_table;
  wabi_map_array new_map;
  wabi_map_iter_t iter;
  wabi_map_entry entry;

  bitmap = WABI_MAP_HASH_BITMAP(map);
  size = WABI_MAP_BITMAP_COUNT(bitmap);
  table = (wabi_map) WABI_MAP_HASH_TABLE(map);
  limit = (wabi_map) (table + size);
  row = table;
  len = 0;
  while(row < limit) {
    tag = WABI_TAG((wabi_val) row);
    if(tag == wabi_tag_map_entry) {
      len++;
      if(len > WABI_MAP_ARRAY_LIMIT)
        return (wabi_map) map;
    } else if (tag == wabi_tag_map_array) {
      len+= WABI_MAP_ARRAY_SIZE((wabi_map_array) row);
      if(len > WABI_MAP_ARRAY_LIMIT)
        return (wabi_map) map;
    } else {
      return (wabi_map) map;
    }
    row++;
  }
  new_map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE * (1 + len));
  if(new_map) {
    new_table = (wabi_map) (new_map + 1);

    row = new_table;
    wabi_map_iterator_init(&iter, (wabi_map) map);
    while((entry = wabi_map_iterator_current(&iter))) {
      *row = (wabi_map_t) *entry;
      wabi_map_iterator_next(&iter);
      row++;
    }
    wabi_map_insert_sort((wabi_map_entry) new_table, size);

    new_map->table = (wabi_word) new_table;;
    new_map->size = len;
    WABI_SET_TAG(new_map, wabi_tag_map_array);
  }
  return (wabi_map) new_map;
}

wabi_map
wabi_map_hash_dissoc_rec(wabi_vm vm,
                         wabi_map_hash map,
                         wabi_val key,
                         wabi_word hash,
                         wabi_word hash_offset)
{
  wabi_word bitmap, size, index, offset;
  wabi_map table, row, sub_map, new_table;
  wabi_map_hash new_map;

  bitmap = WABI_MAP_HASH_BITMAP(map);
  size = WABI_MAP_BITMAP_COUNT(bitmap);
  index = WABI_MAP_HASH_INDEX(hash, hash_offset);
  offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
  table = (wabi_map) WABI_MAP_HASH_TABLE(map);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    row = table + offset;
    sub_map = wabi_map_dissoc_rec(vm, row, key, hash, hash_offset - 6);
    if(!sub_map) {
      new_map = (wabi_map_hash) wabi_vm_alloc(vm, WABI_MAP_SIZE * size);
      if(new_map) {
        new_table = (wabi_map) (new_map + 1);

        memcpy(new_table, table, WABI_MAP_BYTE_SIZE * offset);
        memcpy(new_table + offset, table + offset + 1L, (size - offset - 1L) * WABI_MAP_BYTE_SIZE);
        new_map->bitmap = bitmap ^ (1UL << index);
        new_map->table = (wabi_word) new_table;
        WABI_SET_TAG(new_map, wabi_tag_map_hash);
      }
      else {
        return NULL;
      }
    }
    else {
      new_map = (wabi_map_hash) wabi_vm_alloc(vm, WABI_MAP_SIZE * (size + 1));
      if(new_map) {
        new_table = (wabi_map) (new_map + 1);
        memcpy(new_table, table, size * WABI_MAP_BYTE_SIZE);
        *(new_table + offset) = (wabi_map_t) *sub_map;
        new_map->bitmap = bitmap;
        new_map->table = (wabi_word) new_table;
        WABI_SET_TAG(new_map, wabi_tag_map_hash);
      }
      else {
        return NULL;
      }
    }
    if(size - 1 <= WABI_MAP_ARRAY_LIMIT) {
      return wabi_map_hash_demote(vm, (wabi_map_hash) new_map);
    }
    return (wabi_map) new_map;
  }
  return (wabi_map) map;
}


 wabi_map
wabi_map_dissoc_rec(wabi_vm vm,
                    wabi_map map,
                    wabi_val key,
                    wabi_word hash,
                    wabi_word hash_offset)
{
  wabi_word tag;

  tag = WABI_TAG((wabi_val) map);
  if(tag == wabi_tag_map_entry) {
    return wabi_map_entry_dissoc_rec(vm, (wabi_map_entry) map, key, hash, hash_offset);
  }
  if(tag == wabi_tag_map_array) {
    return wabi_map_array_dissoc_rec(vm, (wabi_map_array) map, key, hash, hash_offset);
  }
  if(tag == wabi_tag_map_hash) {
    return wabi_map_hash_dissoc_rec(vm, (wabi_map_hash) map, key, hash, hash_offset);
  }
  return NULL;
}


wabi_map
wabi_map_dissoc(wabi_vm vm,
                wabi_map map,
                wabi_val key)
{
  wabi_word hash;
  hash = wabi_hash(key);
  return wabi_map_dissoc_rec(vm, map, key, hash, WABI_MAP_INITIAL_OFFSET);
}


/**
 * GET Operation
 */
wabi_val
wabi_map_hash_get_rec(wabi_map_hash map,
                      wabi_val key,
                      wabi_word hash,
                      wabi_word hash_offset);


wabi_val
wabi_map_array_get_rec(wabi_map_array map,
                       wabi_val key,
                       wabi_word hash,
                       wabi_word hash_offset);


wabi_val
wabi_map_hash_get_rec(wabi_map_hash map,
                      wabi_val key,
                      wabi_word hash,
                      wabi_word hash_offset)
{
  wabi_val key0;
  wabi_word bitmap, index, offset, tag;
  wabi_map table, child;

  bitmap = WABI_MAP_HASH_BITMAP(map);
  table = WABI_MAP_HASH_TABLE(map);
  index = WABI_MAP_HASH_INDEX(hash, hash_offset);

  if(WABI_MAP_BITMAP_CONTAINS(bitmap, index)) {
    offset = WABI_MAP_BITMAP_OFFSET(bitmap, index);
    child = table + offset;
    tag = WABI_TAG((wabi_val) child);
    if(tag == wabi_tag_map_entry) {
      key0 = WABI_MAP_ENTRY_KEY((wabi_map_entry) child);
      if(!wabi_cmp(key, key0)) {
        return (wabi_val) WABI_MAP_ENTRY_VALUE((wabi_map_entry) child);
      }
      return NULL;
    }
    if(tag == wabi_tag_map_hash) {
      return wabi_map_hash_get_rec((wabi_map_hash) child, key, hash, hash_offset - 6);
    }
    if(tag == wabi_tag_map_array) {
      return wabi_map_array_get_rec((wabi_map_array) child, key, hash, hash_offset - 6);
    }
  }
  return NULL;
}

wabi_val
wabi_map_array_get_rec(wabi_map_array map,
                       wabi_val key,
                       wabi_word hash,
                       wabi_word hash_offset)
{
  wabi_map_entry table;
  wabi_val key0;
  wabi_size size;
  int first, last, middle;
  int cmp;

  size = WABI_MAP_ARRAY_SIZE(map);
  if(size <= 0) return NULL;

  table = (wabi_map_entry) WABI_MAP_ARRAY_TABLE(map);
  first = 0;
  last = size - 1;
  do {
    middle = (last + first) / 2UL;
    key0 = (wabi_val) WABI_MAP_ENTRY_KEY(table + middle);
    cmp = wabi_cmp(key, key0);
    if(cmp < 0) {
      first = middle + 1;
    }
    else if(cmp > 0) {
      last = middle - 1;
    } else {
      return (wabi_val) WABI_MAP_ENTRY_VALUE(table + middle);
    }
  } while(first <= last);
  return NULL;
}

wabi_val
wabi_map_get_rec(wabi_map map,
                 wabi_val key,
                 wabi_word hash,
                 wabi_word hash_offset)
{
  // todo: convert in a single loop (or exploit TCO opt in GCC?)
  if(WABI_TAG(map) == wabi_tag_map_array) {
    return wabi_map_array_get_rec((wabi_map_array) map, key, hash, hash_offset);
  }
  return wabi_map_hash_get_rec((wabi_map_hash) map, key, hash, hash_offset);
}


wabi_val
wabi_map_get(wabi_map map,
             wabi_val key)
{
  wabi_word hash;
  hash = wabi_hash(key);
  return wabi_map_get_rec(map, key, hash, WABI_MAP_INITIAL_OFFSET);
}


/**
 * Iterator
 */

void
wabi_map_iterator_grow(wabi_map_iter iter)
{
  wabi_map_iter_frame frame;
  wabi_map map, table;
  wabi_word bitmap, size, tag;
  int pos;

  if(iter->top < 0) return;
  do {
    frame = iter->stack + iter->top;
    map = frame->map;
    tag = WABI_TAG((wabi_val) map);
    if(tag == wabi_tag_map_entry) {
      return;
    }
    if(tag == wabi_tag_map_array) {
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
    } else if (tag == wabi_tag_map_hash) {
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
    } else {
      return;
    }
  }
  while(iter->top < WABI_MAP_ITER_STACK_SIZE);
}


int
wabi_map_iterator_frame_full(wabi_map_iter_frame frame)
{
  wabi_word size, bitmap, tag;
  wabi_map map;

  map = frame->map;
  tag = WABI_TAG((wabi_val) map);
  if(tag == wabi_tag_map_entry) {
    return 1;
  }
  if(tag == wabi_tag_map_array) {
    size = WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
    return frame->pos + 1 >= size;
  }
  if(tag == wabi_tag_map_hash) {
    bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
    size = WABI_MAP_BITMAP_COUNT(bitmap);
    return frame->pos + 1 >= size;
  }
  return 0;
}


 void
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
  wabi_map_iter_frame frame;
  frame = iter->stack;
  iter->top = 0;
  frame->map = map;
  frame->pos = 0;
  wabi_map_iterator_grow(iter);
}


wabi_map_entry
wabi_map_iterator_current(wabi_map_iter iter) {
  wabi_map_iter_frame frame;
  if(iter->top < 0) return NULL;
  frame = iter->stack + iter->top;
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

wabi_map
wabi_map_empty(wabi_vm vm)
{
  wabi_map_array map;
  map = (wabi_map_array) wabi_vm_alloc(vm, WABI_MAP_SIZE);
  if(map) {
    map->size = 0UL;
    map->table = 0UL;
    WABI_SET_TAG(map, wabi_tag_map_array);
  }
  return (wabi_map) map;
}


/**
 * Length
 */

wabi_word
wabi_map_length(wabi_map map) {
  wabi_map table, limit;
  wabi_word size, bitmap, tag;

  tag = WABI_TAG((wabi_val) map);
  if(tag == wabi_tag_map_array) {
    return WABI_MAP_ARRAY_SIZE((wabi_map_array) map);
  }
  if(tag == wabi_tag_map_hash) {
    table = WABI_MAP_HASH_TABLE((wabi_map_hash) map);
    bitmap = WABI_MAP_HASH_BITMAP((wabi_map_hash) map);
    limit = table + WABI_MAP_BITMAP_COUNT(bitmap);
    size = 0;
    while(table < limit) {
      size += wabi_map_length(table);
      table++;
    }
    return size;
  }
  if(tag == wabi_tag_map_entry) {
    return 1;
  }
  return 0;
}

static void
wabi_map_builtin_hmap(const wabi_vm vm)
{
  // if arity is odd it raises an error.
  // the alternative is that is associate the last value (key) to
  // nil, but it's an inconsistent behavior, i.e.
  // `(get (hmap "a" 10) "b")` -> `nil`
  //  (get (hmap "a") "a") -> `nil`
  wabi_val ctrl, k, v;
  wabi_map res;
  ctrl = vm->ctrl;
  res = wabi_map_empty(vm);
  if(vm->ert) return;

  while(*ctrl != wabi_val_nil) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      v = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      res = wabi_map_assoc(vm, res, k, v);
      if(vm->ert) return;
    } else {
      vm->ert =  wabi_error_bindings;
      return;
    }
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_map_builtin_assoc(const wabi_vm vm)
{
  wabi_val ctrl, k, v;
  wabi_map res;
  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = (wabi_map) wabi_car((wabi_pair) ctrl);
  ctrl = wabi_cdr((wabi_pair) ctrl);

  switch(WABI_TAG((wabi_val) res)) {
  case wabi_tag_map_array:
  case wabi_tag_map_hash:
  case wabi_tag_map_entry:
    break;
  default:
    vm->ert = wabi_error_type_mismatch;
    return;
  }
  while(*ctrl != wabi_val_nil) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    if(WABI_IS(wabi_tag_pair, ctrl)) {
      v = wabi_car((wabi_pair) ctrl);
      ctrl = wabi_cdr((wabi_pair) ctrl);
      res = wabi_map_assoc(vm, res, k, v);
      if(vm->ert) return;
    } else {
      vm->ert = wabi_error_bindings;
      return;
    }
  }
  vm->ctrl = (wabi_val) res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_map_builtin_dissoc(const wabi_vm vm)
{
  wabi_val ctrl, k, res;
  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  res = wabi_car((wabi_pair) ctrl);
  ctrl  = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_map(res)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  while(WABI_IS(wabi_tag_pair, ctrl)) {
    k = wabi_car((wabi_pair) ctrl);
    ctrl = wabi_cdr((wabi_pair) ctrl);
    res = (wabi_val) wabi_map_dissoc(vm, (wabi_map) res, k);
    if(vm->ert) return;
  }
  if(!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  vm->ctrl = res;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);
}


static void
wabi_map_builtin_map_p(const wabi_vm vm)
{
  wabi_builtin_predicate(vm, &wabi_is_map);
}


static void
wabi_map_builtin_len(const wabi_vm vm)
{
  wabi_val ctrl, k;
  wabi_map m;
  wabi_fixnum len;

  ctrl = vm->ctrl;
  if(!WABI_IS(wabi_tag_pair, ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  m = (wabi_map) wabi_car((wabi_pair) ctrl);
  ctrl  = wabi_cdr((wabi_pair) ctrl);

  if(! wabi_is_map((wabi_val) m)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  if(!wabi_is_nil(ctrl)) {
    vm->ert = wabi_error_bindings;
    return;
  }
  len = wabi_fixnum_new(vm, wabi_map_length(m));
  if(vm->ert) return;

  vm->ctrl = len;
  vm->cont = (wabi_val) wabi_cont_next((wabi_cont) vm->cont);

}


static inline void
wabi_map_hash_entry(wabi_hash_state state, wabi_map_entry entry)
{
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_KEY(entry));
  wabi_hash_val(state, (wabi_val) WABI_MAP_ENTRY_VALUE(entry));
}


static inline void
wabi_map_hash_map(wabi_hash_state state, wabi_map map)
{
  wabi_map_entry entry;
  wabi_map_iter_t iter;
  wabi_map_iterator_init(&iter, map);
  while((entry = wabi_map_iterator_current(&iter))) {
    wabi_map_hash_entry(state, entry);
    wabi_map_iterator_next(&iter);
  }
}


void
wabi_map_hash_(wabi_hash_state state, wabi_map map)
{
  wabi_hash_step(state, "M", 1);
  if(WABI_IS(wabi_tag_map_entry, map)) {
    wabi_map_hash_entry(state, (wabi_map_entry) map);
    return;
  }
  wabi_map_hash_map(state, map);
}


int
wabi_map_cmp(wabi_map left, wabi_map right)
{
  wabi_map_iter_t left_iter, right_iter;
  wabi_map_entry left_entry, right_entry;
  int cmp;
  wabi_map_iterator_init(&left_iter, left);
  wabi_map_iterator_init(&right_iter, right);

  do {
    left_entry = wabi_map_iterator_current(&left_iter);
    right_entry = wabi_map_iterator_current(&right_iter);

    if(!left_entry && !right_entry) {
      return 0;
    }
    else if(!right_entry) {
      return 1;
    }
    else if(!left_entry) {
      return -1;
    }
    else {
      cmp = wabi_cmp(WABI_MAP_ENTRY_KEY(left_entry),
                     WABI_MAP_ENTRY_KEY(right_entry));
      if(cmp) return cmp;
      cmp =  wabi_cmp(WABI_MAP_ENTRY_VALUE(left_entry),
                      WABI_MAP_ENTRY_VALUE(right_entry));
      if(cmp) return cmp;
    }
    wabi_map_iterator_next(&left_iter);
    wabi_map_iterator_next(&right_iter);
  } while(1);
}


void
wabi_map_builtins(const wabi_vm vm, const wabi_env env)
{
  wabi_defx(vm, env, "hmap", &wabi_map_builtin_hmap);
  if(vm->ert) return;

  wabi_defn(vm, env, "assoc", &wabi_map_builtin_assoc);
  if(vm->ert) return;

  wabi_defn(vm, env, "dissoc", &wabi_map_builtin_dissoc);
  if(vm->ert) return;

  wabi_defn(vm, env, "map-len", &wabi_map_builtin_len);
  if(vm->ert) return;

  wabi_defn(vm, env, "map?", &wabi_map_builtin_map_p);
  if(vm->ert) return;

  /* res = WABI_DEFN(vm, env, "map-merge", "mamerge?", wabi_map_builtin_map_merge); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "map-keys", "ma-keys", wabi_map_builtin_map_keys); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "map-values", "ma-values", wabi_map_builtin_map_values); */
  /* if(res) return res; */
  /* res = WABI_DEFN(vm, env, "map-take", "ma-take", wabi_map_builtin_map_take); */
  /* if(res) return res; */
}
