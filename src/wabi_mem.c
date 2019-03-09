/**
 * Memory manager
 */

#define wabi_mem_c

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

void
wabi_mem_copy(wabi_word_t *src, wabi_word_t **dst)
{
  wabi_word_t tag = wabi_obj_tag(src);
  if(tag == WABI_TAG_FORWARD) {
    *dst = (wabi_word_t *) (*src & WABI_VALUE_MASK);
    return;
  }

  *dst = wabi_mem_alloc;

  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    **dst = *src;
    wabi_mem_alloc++;
  } else if(tag == WABI_TAG_PAIR) {
    **dst = *src;
    *(*dst + 1) = *(src + 1);
    wabi_mem_alloc += 2;
  }
  *src = WABI_TAG_FORWARD | ((wabi_word_t) *dst);
}

void
wabi_mem_collect_step()
{
  wabi_word_t tag = wabi_obj_tag(wabi_mem_scan);
  if(tag <= WABI_TAG_ATOMIC_LIMIT) {
    wabi_mem_scan++;
  }
  else if(tag == WABI_TAG_PAIR) {
    wabi_word_t *car, *cdr;
    wabi_mem_copy((wabi_word_t *) (*wabi_mem_scan & WABI_VALUE_MASK), &car);
    wabi_mem_copy((wabi_word_t *) *(wabi_mem_scan + 1), &cdr);
    *wabi_mem_scan = WABI_TAG_PAIR | (wabi_word_t) car;
    *(wabi_mem_scan + 1) = (wabi_word_t) cdr;
    wabi_mem_scan += 2;
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

  wabi_mem_copy(wabi_mem_root, &wabi_mem_root);

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
