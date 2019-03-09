/**
 * Memory manager
 */

#define wabi_mem_c

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "wabi_object.h"
#include "wabi_mem.h"
#include "wabi_err.h"

#define WABI_MEM_LIMIT (wabi_word_t *)0x00FFFFFFFFFFFFFF

wabi_word_t *
wabi_allocate_space()
{
  return (wabi_word_t *) malloc(WABI_WORD_SIZE * wabi_mem_size);
}

void
wabi_mem_init(wabi_size_t size, int* errno)
{
  wabi_mem_size = size;
  wabi_mem_from_space = wabi_allocate_space();

  if(wabi_mem_from_space == NULL) {
    *errno = WABI_ERROR_NOMEM;
    return;
  }
  if(wabi_mem_from_space + wabi_mem_size > WABI_MEM_LIMIT) {
    *errno = WABI_ERROR_NOMEM;
    return;
  }

  wabi_mem_limit = wabi_mem_from_space + wabi_mem_size;
  wabi_mem_alloc = wabi_mem_from_space;
  wabi_mem_scan = NULL;
}

wabi_word_t *
wabi_mem_copy(wabi_word_t *obj)
{
  if(wabi_obj_is_forward(obj)) {
    return (wabi_word_t *)wabi_obj_value(obj);
  }

  wabi_word_t *new_obj = wabi_mem_alloc;
  wabi_size_t size = wabi_obj_size(obj);
  memcpy(new_obj, obj, size * WABI_WORD_SIZE);
  wabi_mem_alloc += size;

  *obj = WABI_TAG_FORWARD | ((wabi_word_t) new_obj);

  return new_obj;
}

void
wabi_mem_collect_step()
{
  wabi_word_t tag = wabi_obj_tag(wabi_mem_scan);

  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    wabi_mem_scan++;
    return;
  }

  if(tag == WABI_TAG_PAIR) {
    wabi_word_t *car = (wabi_word_t *)(*wabi_mem_scan & WABI_VALUE_MASK);
    wabi_word_t *cdr = (wabi_word_t *)(*(wabi_mem_scan + 1) & WABI_VALUE_MASK);

    *cdr = (wabi_word_t) wabi_mem_copy(cdr);
    *car = (wabi_word_t) wabi_mem_copy(car);
    wabi_mem_scan+=2;
  }
}

void
wabi_mem_collect(int *errno)
{
  wabi_word_t *wabi_mem_to_space = wabi_mem_from_space;
  wabi_mem_from_space = wabi_allocate_space();

  if(wabi_mem_from_space == NULL) {
    *errno = WABI_ERROR_NOMEM;
    return;
  }
  if(wabi_mem_from_space + wabi_mem_size > WABI_MEM_LIMIT) {
    *errno = WABI_ERROR_NOMEM;
    return;
  }
  wabi_mem_limit = wabi_mem_from_space + wabi_mem_size;
  wabi_mem_alloc = wabi_mem_from_space;
  wabi_mem_scan = wabi_mem_from_space;

  wabi_mem_root = wabi_mem_copy(wabi_mem_root);
  printf("root: %lx\n", *wabi_mem_root);
  while(wabi_mem_scan < wabi_mem_alloc)
    wabi_mem_collect_step();

  free(wabi_mem_to_space);
}

void
wabi_mem_allocate(wabi_size_t size, wabi_word_t **res, int *errno)
{
  if(wabi_mem_alloc + size >= wabi_mem_limit)
    wabi_mem_collect(errno);

  if(wabi_mem_alloc + size >= wabi_mem_limit) {
    *errno = WABI_ERROR_NOMEM;
    return;
  }
  *res = wabi_mem_alloc;
  wabi_mem_alloc += size;
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
