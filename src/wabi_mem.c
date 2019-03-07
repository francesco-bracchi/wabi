/**
 * Memory manager
 */

#define wabi_mem_c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>

#include "wabi_types.h"
#include "wabi_mem.h"
#include "wabi_err.h"

wabi_size_t wabi_mem_size;
wabi_word_t *wabi_mem_from_space;
wabi_word_t *wabi_mem_to_space;
wabi_word_t *wabi_mem_limit;
wabi_word_t *wabi_mem_alloc;
wabi_word_t *wabi_mem_scan;
wabi_word_t *wabi_mem_root;

#define WABI_MEM_PROT PROT_READ | PROT_WRITE
#define WABI_MEM_FLAGS MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE

wabi_word_t *
wabi_mem_alloc_hemispace(wabi_word_t *from, wabi_size_t size)
{
  return (wabi_word_t *)
    mmap(from, size, WABI_MEM_PROT, WABI_MEM_FLAGS, -1, 0);
}


void
wabi_mem_free_hemispace(wabi_word_t *from, wabi_size_t size)
{
  munmap(from, size);
}

void
wabi_mem_init(wabi_size_t size)
{
  wabi_mem_size = size;
  wabi_mem_from_space =
    (wabi_word_t *) wabi_mem_alloc_hemispace((wabi_word_t *) WABI_MEM_FROM, WABI_MEM_HALF);

  if(wabi_mem_from_space == MAP_FAILED) {
    wabi_errno = WABI_ERROR_NOMEM;
    return;
  }

  wabi_mem_to_space = wabi_mem_alloc_hemispace((wabi_word_t *) WABI_MEM_TO - WABI_MEM_HALF, WABI_MEM_HALF);
  if(wabi_mem_to_space == MAP_FAILED) {
    wabi_errno = WABI_ERROR_NOMEM;
    return;
  }

  wabi_mem_limit = wabi_mem_from_space + size;
  wabi_mem_alloc = wabi_mem_from_space;
  wabi_mem_scan = NULL;

  return;
}

wabi_word_t *
wabi_mem_copy(wabi_word_t *obj)
{
  if(wabi_is_forward(obj)) {
    printf("it's forward\n");
    return (wabi_word_t*) (*obj & WABI_TYPE_VALUE_MASK);
  }

  wabi_word_t *new_obj = wabi_mem_alloc;
  wabi_size_t size = wabi_type_size(obj);
  memcpy(new_obj, obj, size * WABI_WORD_SIZE);
  wabi_mem_alloc += size;

  *obj = WABI_TYPE_TAG_FORWARD | ((wabi_word_t) new_obj & WABI_TYPE_VALUE_MASK);

  return new_obj;
}

void
wabi_mem_collect_step()
{
  wabi_word_t* new_obj = wabi_mem_scan;
  wabi_word_t tag = wabi_type_tag(new_obj);

  if(tag < WABI_TYPE_TAG_PAIR) {
    wabi_mem_scan++;
    return;
  }

  if(tag == WABI_TYPE_TAG_PAIR) {
    wabi_pair_t *pair = (wabi_pair_t*) new_obj;

    wabi_word_t *cdr_ptr = (wabi_word_t*) (pair->cdr & WABI_TYPE_VALUE_MASK);
    wabi_word_t *new_cdr_ptr = wabi_mem_copy(cdr_ptr);
    pair->cdr = (wabi_word_t) new_cdr_ptr;

    wabi_word_t *car_ptr = (wabi_word_t*) (pair->car & WABI_TYPE_VALUE_MASK);
    wabi_word_t *new_car_ptr = wabi_mem_copy(car_ptr);
    pair->car = ((wabi_word_t) new_car_ptr) | WABI_TYPE_TAG_PAIR;

    wabi_mem_scan+=2;
  }
}

void
wabi_mem_collect()
{
  wabi_word_t *to_space = wabi_mem_from_space;
  wabi_mem_from_space = wabi_mem_to_space;
  wabi_mem_limit = wabi_mem_from_space + wabi_mem_size;
  wabi_mem_alloc = wabi_mem_from_space;
  wabi_mem_scan = wabi_mem_from_space;

  wabi_mem_root = wabi_mem_copy(wabi_mem_root);

  while(wabi_mem_scan < wabi_mem_alloc)
    wabi_mem_collect_step();

  wabi_mem_free_hemispace(to_space, WABI_MEM_HALF);
  wabi_mem_alloc_hemispace(to_space, WABI_MEM_HALF);
  wabi_mem_to_space = to_space;
}

wabi_word_t *
wabi_mem_allocate(wabi_size_t size)
{
  if(wabi_mem_alloc + size >= wabi_mem_limit) {
    wabi_mem_collect();

    if(wabi_errno > 0)
      return NULL;
  }
  if(wabi_mem_alloc + size >= wabi_mem_limit) {
    if(wabi_errno > 0) {
      wabi_errno = WABI_ERROR_NOMEM;
      return NULL;
    }
  }

  wabi_word_t *res = wabi_mem_alloc;
  wabi_mem_alloc += size;
  return res;
}

wabi_word_t
wabi_mem_used()
{
  // if not running gc!
  return (wabi_word_t)(wabi_mem_alloc - wabi_mem_from_space);
}

wabi_word_t
wabi_mem_total()
{
  // if not running gc!
  return (wabi_word_t)(wabi_mem_alloc - wabi_mem_from_space);
}
