/**
 * Memory manager
 */

#ifndef wabi_mem_h
#define wabi_mem_h

#include "wabi_object.h"

wabi_word_t *wabi_mem_root;

void
wabi_mem_init(wabi_size_t size, int* errno);

void
wabi_mem_allocate(wabi_size_t size, wabi_word_t **res, int *errno);

wabi_size_t
wabi_mem_used();

wabi_size_t
wabi_mem_total();

void
wabi_mem_collect();

wabi_size_t wabi_mem_size;
wabi_word_t *wabi_mem_from_space;
wabi_word_t *wabi_mem_limit;
wabi_word_t *wabi_mem_alloc;
wabi_word_t *wabi_mem_scan;

/* typedef struct wabi_mem_struct */
/* { */
/*   wabi_size_t wabi_mem_size; */
/*   wabi_word_t *wabi_mem_from_space; */
/*   wabi_word_t *wabi_mem_limit; */
/*   wabi_word_t *wabi_mem_alloc; */
/*   wabi_word_t *wabi_mem_scan; */
/*   void* allocate_fn */
/* } wabi_mem_t; */


/* wabi_mem_t *wabi_mem; */

/* void */
/* wabi_mem_init(wabi_mem_t *mem, wabi_size_t size, int* errno); */

/* void */
/* wabi_mem_alloc(wabi_size_t size, wabi_word_t **res, int* errno); */

#endif
