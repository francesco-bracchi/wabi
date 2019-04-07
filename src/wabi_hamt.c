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


const wabi_word_t one = 1;

#define MAP_BITMAP(map) ((map)->bitmap)
#define MAP_TABLE(map) ((wabi_hamt_table) ((map)->table & WABI_VALUE_MASK))
#define BITMAP_OFFSET(bitmap, index) WABI_POPCNT((bitmap) << (64 - (index)))
#define BITMAP_FOUND(bitmap, index) (((bitmap) >> (index)) & 1)
#define BITMAP_SIZE(bitmap) WABI_POPCNT(bitmap)
#define HASH_INDEX(hash, h_pos) (((hash) >> h_pos) & 0x3F)

wabi_hamt_table
wabi_hamt_get_table(wabi_hamt_map map, wabi_hamt_index index)
{
  wabi_word_t bitmap = MAP_BITMAP(map);
  wabi_hamt_table table = MAP_TABLE(map);
  if(BITMAP_FOUND(bitmap, index)) {
    wabi_hamt_index offset = BITMAP_OFFSET(bitmap, index);
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

  new_map->bitmap = bitmap | (one << index);
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


wabi_hamt_entry
wabi_hamt_get_entry(wabi_hamt_map map, wabi_hamt_index h_pos, wabi_word_t hash)
{
  wabi_word_t index;
  wabi_hamt_table row;

  do {
    index = HASH_INDEX(hash, h_pos);
    row = wabi_hamt_get_table(map, index);
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
    map->bitmap = (one << index) | (one << index0);
    map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP;
    return map;
  }
  wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, 3 * WABI_HAMT_SIZE);
  if(vm->errno) return NULL;

  wabi_hamt_table table = (wabi_hamt_table) (map + 1);

  // if(index < index0) {
  if(index < index0) {
    *table = (wabi_hamt_table_t) *entry;
    *(table + 1) = (wabi_hamt_table_t) *entry0;

    map->bitmap = (one << index) | (one << index0);
    map->table = table;
    return map;
  }
  else {
    *table = (wabi_hamt_table_t) *entry0;
    *(table + 1) = (wabi_hamt_table_t) *entry;

    map->bitmap = (one << index) | (one << index0);
    map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP;
    return map;
  }
}


wabi_hamt_map
wabi_hamt_set_entry(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index h_pos, wabi_word_t hash, wabi_hamt_entry entry)
{
  wabi_hamt_index index = (hash >> h_pos) & 0x3F;
  wabi_hamt_table row = wabi_hamt_get_table(map, index);

  if(row) {
    if(wabi_obj_is_hamt_map((wabi_obj) row)) {
      wabi_hamt_map submap =  wabi_hamt_set_entry(vm, (wabi_hamt_map) row, h_pos - 6, hash, entry);
      return wabi_hamt_table_update(vm, map, index, (wabi_hamt_table) submap);
    }
    wabi_hamt_entry entry0 = (wabi_hamt_entry) row;
    wabi_word_t hash0 = wabi_hash_raw((wabi_obj) entry0->key);
    wabi_hamt_map submap = wabi_hamt_merge(vm, h_pos - 6, hash, entry, hash0, entry0);
    printf("ENTRY\n");
    wabi_pr(submap);
    printf("\n");
    return wabi_hamt_table_update(vm, map, index, (wabi_hamt_table) submap);
  }
  return wabi_hamt_table_add(vm, map, index, (wabi_hamt_table) entry);
}


wabi_obj
wabi_hamt_empty(wabi_vm vm) {
  wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE);
  if(vm->errno) return NULL;
  map->bitmap = 0;
  map->table = (wabi_word_t) (map + 1) | WABI_TAG_HAMT_MAP;
  return (wabi_obj) map;
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
  wabi_hamt_entry_t entry;
  entry.key = (wabi_word_t) key;
  entry.value = (wabi_word_t) val | WABI_TAG_HAMT_ENTRY;
  wabi_word_t hash = wabi_hash_raw(key);
  if(vm->errno) return NULL;
  return (wabi_obj) wabi_hamt_set_entry(vm, (wabi_hamt_map) map, (wabi_hamt_index) 50, hash, &entry);
}


void
print_pr_map(wabi_hamt_map map)
{
  wabi_word_t bitmap = MAP_BITMAP(map);
  wabi_hamt_table table = MAP_TABLE(map);
  wabi_hamt_index size = BITMAP_SIZE(bitmap);
  wabi_hamt_table row;
  printf("MAP %p size: %i\n", map, size);
  for(wabi_word_t j = 0; j < 64; j++) {
    row = wabi_hamt_get_table(map, j);
    if(row) {
      if(wabi_obj_is_hamt_map((wabi_obj) row)) {
        printf("MAP %i: %lx %lx\n", j, row->map.table, row->map.bitmap);
      } else {
        printf("ENT %i: %lx %lx | ", j, row->entry.value, row->entry.key);
        wabi_pr(&row->entry);
        printf("\n");
      }
    }
  }
}




/* #define WABI_HAMT_BYTE_SIZE WABI_WORD_SIZE * WABI_HAMT_SIZE */

/* const wabi_word_t one = 1; */


/* /\* wabi_hamt_table *\/ */
/* /\* wabi_hamt_get_table(wabi_hamt_map map, wabi_hamt_word_t index) *\/ */
/* /\* { *\/ */
/* /\*   wabi_word_t bitmap = map->bitmap; *\/ */
/* /\*   wabi_word_t found = (bitmap >> index) & 1; *\/ */
/* /\*   if(! found) return NULL; *\/ */

/* /\*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); *\/ */
/* /\*   wabi_word_t offset = WABI_POPCNT(bitmap >> index + 1) *\/ */
/* /\* } *\/ */


/* wabi_hamt_map */
/* wabi_hamt_set_table(wabi_vm vm, wabi_hamt_map map, wabi_word_t index, wabi_hamt_table row) */
/* { */
/*   wabi_word_t bitmap = map->bitmap; */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_word_t size = WABI_POPCNT(bitmap); */
/*   wabi_word_t offset = WABI_POPCNT(bitmap >> (63 - index)); */
/*   wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, (2 + size) * WABI_HAMT_SIZE); */
/*   if(vm->errno) return NULL; */
/*   wabi_hamt_table new_table = (wabi_hamt_table) (new_map + 1); */

/*   memcpy(new_table, table, WABI_WORD_SIZE * WABI_HAMT_SIZE * offset); */
/*   *(new_table + offset) = (wabi_hamt_table_t) *row; */
/*   memcpy(new_table + offset + 1, table + offset, WABI_HAMT_BYTE_SIZE * (size - offset)); */

/*   new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP; */
/*   new_map->bitmap = bitmap | (one << (63 - index)); */

/*   /\* print_bin_table(map); *\/ */
/*   /\* printf("\n"); *\/ */
/*   /\* print_bin_table(new_map); *\/ */
/*   /\* printf("\n"); *\/ */
/*   return new_map; */
/* } */


/* wabi_hamt_map */
/* wabi_hamt_map_merge(wabi_vm vm, */
/*                     wabi_word_t hash0, */
/*                     wabi_hamt_entry entry0, */
/*                     wabi_word_t hash1, */
/*                     wabi_hamt_entry entry1, */
/*                     wabi_word_t level) */
/* { */
/*   wabi_word_t shift = 64 - level - 6; */
/*   wabi_word_t index0 = (hash0 >> shift) & 0x3F; */
/*   wabi_word_t index1 = (hash1 >> shift) & 0x3F; */

/*   if(index0 == index1) { */
/*     printf("RARE\n"); */
/*     wabi_word_t bitmap = one << (63 - index0); */
/*     wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, 2 * WABI_HAMT_SIZE); */
/*     if(vm->errno) return NULL; */
/*     wabi_hamt_table table = (wabi_hamt_table) (map + 1); */

/*     wabi_hamt_map sub_map = wabi_hamt_map_merge(vm, hash0, entry0, hash1, entry1, level + 6); */
/*     if(vm->errno) return NULL; */

/*     *table = (wabi_hamt_table_t) *sub_map; */
/*     map->bitmap = bitmap; */
/*     map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP; */
/*     return map; */
/*   } */

/*   wabi_word_t bitmap = (one << (63 - index0)) | (one << (63 - index1)); */
/*   wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, 3 * WABI_HAMT_SIZE); */
/*   wabi_hamt_table table = map + 1; */
/*   if(vm->errno) return NULL; */
/*   if(index0 < index1) { */
/*     *table = (wabi_hamt_table_t) *entry0; */
/*     *(table + 1) = (wabi_hamt_table_t) *entry1; */
/*   } else  { */
/*     *table = (wabi_hamt_table_t) *entry1; */
/*     *(table + 1) = (wabi_hamt_table_t) *entry0; */
/*   } */
/*   map->bitmap = bitmap; */
/*   map->table = (wabi_word_t) table | WABI_TAG_HAMT_MAP; */
/*   /\* printf("\n"); *\/ */
/*   /\* print_bin_table(map); *\/ */
/*   /\* printf("\n"); *\/ */
/*   return map; */
/* } */


/* wabi_hamt_map */
/* wabi_hamt_map_set_entry(wabi_vm vm, wabi_hamt_map map, wabi_word_t hash, wabi_word_t level, wabi_hamt_entry entry) */
/* { */
/*   wabi_word_t bitmap = map->bitmap; */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_word_t size = WABI_POPCNT(bitmap); */
/*   wabi_word_t shift = 64 - level - 6; */
/*   wabi_word_t index = (hash >> shift) & 0x3F; */
/*   wabi_word_t found = (bitmap >> index) & 1; */
/*   wabi_word_t offset = WABI_POPCNT(bitmap >> index); */
/*   /\* print_bitmap(hash); *\/ */
/*   /\* printf("\n"); *\/ */
/*   /\* print_bitmap(hash >> shift); *\/ */
/*   /\* printf("\nshift %i index:%i offset: %i found: %i\n", shift, index, offset, found); *\/ */
/*   if(found) { */
/*     // print_bitmap(bitmap); */
/*     wabi_hamt_table row = table + offset; */
/*     if(wabi_obj_is_hamt_map((wabi_obj) row)) { */
/*       printf("FOUND MAP\n"); */
/*       wabi_hamt_table submap = wabi_hamt_map_set_entry(vm, (wabi_hamt_map) row, hash, level + 6, entry); */
/*       if(vm->errno) return NULL; */
/*       return wabi_hamt_set_table(vm, map, index, (wabi_hamt_table) submap); */
/*     } else { */
/*       // printf("FOUND ENTRY size: %lu, offset: %lu, shift: %lu\n", size, offset, shift); */
/*       wabi_hamt_entry entry0 = (wabi_hamt_entry) row; */
/*       wabi_word_t hash0 = wabi_hash_raw(entry0->key); */
/*       wabi_hamt_map sub_map = wabi_hamt_map_merge(vm, hash, entry, hash0, entry0, level + 6); */
/*       wabi_hamt_map new_map = wabi_hamt_set_table(vm, map, index, (wabi_hamt_table) sub_map); */
/*       /\* wabi_word_t *x = (wabi_word_t*) ((new_map->table & WABI_VALUE_MASK) + offset); *\/ */
/*       /\* wabi_word_t *y = (wabi_word_t*) sub_map; *\/ */
/*       /\* printf("NM0 %lx|%lx\n", *x, *(x + 1)); *\/ */
/*       /\* printf("SM0 %lx|%lx\n", *y, *(y + 1)); *\/ */
/*       /\* printf("OLD MAP\n"); *\/ */
/*       /\* print_bin_table(map); *\/ */
/*       /\* printf("\nSUB MAP\n"); *\/ */
/*       /\* print_bin_table(sub_map); *\/ */
/*       /\* printf("\nNEW MAP\n"); *\/ */
/*       /\* print_bin_table(new_map); *\/ */
/*       /\* printf("\n----------------\n"); *\/ */
/*       return new_map; */

/*     } */
/*   } */
/*   else { */
/*     // printf("FREE\n"); */
/*     return wabi_hamt_set_table(vm, map, index, (wabi_hamt_table) entry); */
/*   } */
/* } */


/* wabi_obj */
/* wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val) */
/* { */
/*   if(wabi_obj_is_nil(map)) { */
/*     map = wabi_hamt_empty(vm); */
/*     if(vm->errno) return NULL; */
/*   } */

/*   if(!wabi_obj_is_hamt_map(map)) { */
/*     vm->errno = WABI_ERROR_TYPE_MISMATCH; */
/*     return NULL; */
/*   } */
/*   wabi_hamt_entry_t entry; */
/*   entry.key = (wabi_word_t) key; */
/*   entry.value = (wabi_word_t) val | WABI_TAG_HAMT_ENTRY; */

/*   wabi_word_t hash = wabi_hash_raw(key); */
/*   if(vm->errno) return NULL; */
/*   return (wabi_obj) wabi_hamt_map_set_entry(vm, (wabi_hamt_map) map, hash, 8, &entry); */
/* } */


/* wabi_obj */
/* wabi_hamt_empty(wabi_vm vm) { */
/*   wabi_hamt_map map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE); */
/*   map->bitmap = 0; */
/*   map->table = WABI_TAG_HAMT_MAP; */
/*   return (wabi_obj) map; */
/* } */


/* int64_t */
/* wabi_hamt_length_raw(wabi_hamt_map map) */
/* { */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_size_t size = WABI_POPCNT(map->bitmap); */
/*   int64_t total = 0; */
/*   for(int j = 0; j < size; j++) { */
/*     wabi_hamt_table row = table + j; */
/*     if(wabi_obj_is_hamt_map((wabi_obj) row)) { */
/*       total += wabi_hamt_length_raw((wabi_hamt_map) row); */
/*     } else { */
/*       total++; */
/*     } */
/*   } */
/*   return total; */
/* } */

/* wabi_obj */
/* wabi_hamt_length(wabi_vm vm, wabi_obj map) */
/* { */

/*   if(wabi_obj_is_nil(map)) */
/*     return wabi_smallint(vm, 0); */

/*   if(!wabi_obj_is_hamt_map(map)) { */
/*     vm->errno = WABI_ERROR_TYPE_MISMATCH; */
/*     return NULL; */
/*   } */
/*   int64_t length = wabi_hamt_length_raw(map); */
/*   return wabi_smallint(vm, length); */
/* } */

/* // ---------------------------------------------------------------- */

/* void */
/* print_bitmap(wabi_word_t t) */
/* { */
/*   for(int j = 0; j < 64; j++){ */
/*     putchar(wabi_hamt_bit_get(t, j) ? '1' : '_'); */
/*     // printf(" "); */
/*   } */
/* } */


/* void */
/* print_hamt_map(wabi_hamt_map map) */
/* { */
/*   wabi_word_t size = WABI_POPCNT(map->bitmap); */
/*   wabi_hamt_table table = (wabi_hamt_table)(map->table & WABI_VALUE_MASK); */
/*   for(int j = 0; j < size; j++) { */
/*     if(1) { */

/*       wabi_hamt_entry t = (wabi_hamt_entry) table + j; */
/*       // printf("Entry tag: %lx %lx %lx\n", t->value >> 56, t->value & WABI_VALUE_MASK, t->key); */
/*       wabi_obj key = (wabi_obj) t->key; */
/*       printf("Key: %p ", t->key); */
/*       wabi_pr(key); */
/*       printf("\nValue: %p ", t->value & WABI_VALUE_MASK); */
/*       wabi_pr(t->value & WABI_VALUE_MASK); */
/*       printf("\n"); */
/*     } */
/*   } */
/* } */

/* wabi_hamt_table */
/* wabi_hamt_get_table(wabi_hamt_map map, wabi_hamt_index index) */
/* { */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_word_t mask = map->bitmap; */
/*   mask >>= 63 - index; */
/*   if(mask & 1)  { */
/*     // printf("GOTCHA\n"); */
/*     wabi_word_t diff = WABI_POPCNT(mask); */
/*     wabi_hamt_table entry0 = table + diff; */
/*     return entry0; */
/*   } */
/*   return NULL; */
/* } */

/* wabi_hamt_map */
/* wabi_hamt_set_table(wabi_vm vm, wabi_hamt_map map, wabi_hamt_index index, wabi_hamt_table elem) */
/* { */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_word_t size = WABI_POPCNT(map->bitmap); */
/*   wabi_word_t pivot = wabi_hamt_table_offset(map->bitmap, index); */
/*   wabi_hamt_table new_table = (wabi_hamt_table) wabi_mem_allocate(vm, WABI_HAMT_SIZE * (size + 1)); */
/*   if(vm->errno) return NULL; */

/*   memcpy(new_table, table, WABI_WORD_SIZE * WABI_HAMT_SIZE * pivot); */
/*   *(new_table + pivot) = *elem; */
/*   memcpy(new_table + pivot + 1, table + pivot, WABI_WORD_SIZE * WABI_HAMT_SIZE * (size - pivot)); */

/*   wabi_hamt_map new_map = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE); */
/*   if(vm->errno) return NULL; */

/*   new_map->table = (wabi_word_t) new_table | WABI_TAG_HAMT_MAP; */
/*   wabi_word_t mask = 1; */
/*   mask <<= (63 - index); */
/*   new_map->bitmap = map->bitmap | mask; */

/*   /\* printf("after: "); *\/ */
/*   /\* print_bitmap(new_map->bitmap); *\/ */
/*   /\* printf("\n"); *\/ */

/*   return new_map; */
/* } */


/* wabi_hamt_entry */
/* wabi_hamt_get_entry(wabi_hamt_map map, */
/*                     wabi_hamt_index offset, */
/*                     wabi_obj key, */
/*                     wabi_word_t hash) */
/* { */
/*   wabi_word_t index; */
/*   wabi_hamt_table table; */

/*   do { */
/*     index = wabi_hamt_index(hash, offset); */
/*     table = wabi_hamt_get_table(map, index); */
/*     if(table && wabi_hamt_is_map(table)) { */
/*       map = (wabi_hamt_map) table; */
/*       offset += 6; */
/*       continue; */
/*     } */
/*     return (wabi_hamt_entry) table; */
/*   } while(offset < 64); // todo use the constant */
/*   return NULL; */
/* } */


/* wabi_hamt_map */
/* wabi_hamt_set_entry(wabi_vm vm, */
/*                     wabi_hamt_map map, */
/*                     wabi_hamt_index offset, */
/*                     wabi_obj key, */
/*                     wabi_word_t hash, */
/*                     wabi_hamt_entry entry) */
/* { */
/*   wabi_hamt_map submap; */
/*   wabi_hamt_index index = wabi_hamt_index(hash, offset); */
/*   wabi_hamt_table table = (wabi_hamt_table) (map->table & WABI_VALUE_MASK); */
/*   wabi_hamt_table entry0 = wabi_hamt_get_table(map, index); */

/*   if(entry0) { */
/*     if(wabi_hamt_is_map(entry0)) { */
/*       submap = (wabi_hamt_table_t *) entry0; */
/*       submap = wabi_hamt_set_entry(vm, submap, offset + 6, key, hash, entry); */
/*       if(vm->errno) return NULL; */
/*       return wabi_hamt_set_table(vm, map, index, submap); */
/*     } else { */
/*       // printf("ENT "); */
/*       wabi_hamt_map submap = (wabi_hamt_map) wabi_hamt_empty(vm); */
/*       if(vm->errno) return NULL; */

/*       submap = (wabi_hamt_table) wabi_hamt_set_entry(vm, submap, offset + 6, key, hash , entry0); */
/*       submap = (wabi_hamt_table) wabi_hamt_set_entry(vm, submap, offset + 6, key, hash , entry); */
/*       if(vm->errno) return NULL; */

/*       return wabi_hamt_set_table(vm, map, index, submap); */
/*     } */
/*   } */
/*   // printf("EMP "); */
/*   /\* printf("I %i \n", index); *\/ */
/*   return wabi_hamt_set_table(vm, map, index, (wabi_hamt_table) entry); */
/* } */


/* wabi_hamt_map */
/* wabi_hamt_set_raw(wabi_vm vm, wabi_hamt_map map, wabi_obj key, wabi_obj value) */
/* { */
/*   wabi_word_t hash = wabi_hash_raw(key); */
/*   wabi_hamt_entry_t entry; */
/*   entry.key = (wabi_word_t) key; */
/*   entry.value = (wabi_word_t) value; */

/*   return wabi_hamt_set_entry(vm, map, 0, key, hash, &entry); */
/* } */


/* wabi_obj */
/* wabi_hamt_set(wabi_vm vm, wabi_obj map, wabi_obj key, wabi_obj val) */
/* { */
/*   if(wabi_obj_is_nil(map)) { */
/*     map = wabi_hamt_empty(vm); */
/*     if(vm->errno) return NULL; */
/*   } */
/*   if(!wabi_obj_is_hamt_map(map)) { */
/*     vm->errno = WABI_ERROR_TYPE_MISMATCH; */
/*     return NULL; */
/*   } */
/*   return (wabi_obj) wabi_hamt_set_raw(vm, (wabi_hamt_map) map, key, val); */
/* } */


/* wabi_obj */
/* wabi_hamt_get_raw(wabi_hamt_map map, wabi_obj key) */
/* { */
/*   wabi_word_t hash = wabi_hash_raw(key); */
/*   wabi_hamt_entry entry =  wabi_hamt_get_entry(map, 0, key, hash); */

/*   return entry ? (wabi_obj) entry->value : NULL; */
/* } */


/* wabi_obj */
/* wabi_hamt_get(wabi_vm vm, wabi_obj map, wabi_obj key) */
/* { */
/*   if(!wabi_obj_is_hamt_map(map)) { */
/*     vm->errno = WABI_ERROR_TYPE_MISMATCH; */
/*     return NULL; */
/*   } */
/*   wabi_obj res =  wabi_hamt_get_raw((wabi_hamt_map) map, key); */
/*   if(res) return res; */
/*   return wabi_nil(vm); */
/* } */


/* wabi_obj */
/* wabi_hamt_empty(wabi_vm vm) */
/* { */
/*   wabi_hamt_map res = (wabi_hamt_map) wabi_mem_allocate(vm, WABI_HAMT_SIZE); */
/*   if(vm->errno) return NULL; */

/*   res->bitmap = 0; */
/*   res->table = WABI_TAG_HAMT_MAP; */
/*   return (wabi_obj) res; */
/* } */
