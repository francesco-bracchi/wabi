#define wabi_reader_c

#include <unistd.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_pr.h"

#define WABI_READER_BUFFER_SIZE 1024
#define WABI_READ_WORD_SIZE 512
#define WABI_READ_BINARY_SIZE 512


static inline void
wabi_next(FILE *fd,
          char* cur)
{
  *cur = getc(fd);

}


static inline void
wabi_whitespace(FILE *fd,
                char* cur)
{
  do {
    switch(*cur) {
    case ' ':
    case '\n':
    case '\t':
      wabi_next(fd, cur);
      continue;
    default: return;
    }
  }
  while(1);
}


static wabi_val
wabi_read_rec(wabi_vm vm,
              FILE *fd,
              char* cur);


static wabi_val
wabi_read_list(wabi_vm vm,
               FILE *fd,
               char* cur)
{
  wabi_val car, cdr;
  wabi_whitespace(fd, cur);
  car = wabi_read_rec(vm, fd, cur);
  if(vm->errno) return NULL;
  wabi_whitespace(fd, cur);

  switch(*cur) {
  case ')':
    wabi_next(fd, cur);
    cdr = wabi_nil(vm);
    break;
  case '.':
    wabi_next(fd, cur);
    cdr = wabi_read_rec(vm, fd, cur);
    break;
  default:
    cdr = wabi_read_list(vm, fd, cur);
    break;
  }
  if(vm->errno) return NULL;
  return wabi_cons(vm, car, cdr);
}


static wabi_binary
wabi_read_binary(wabi_vm vm,
                 FILE *fd,
                 char* cur)
{
  char *blob;
  size_t size;
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_binary_new(vm, WABI_READ_WORD_SIZE);
  if(vm->errno) return NULL;
  blob = (char*) leaf->data_ptr;
  size = 0;
  while(size < WABI_READ_WORD_SIZE) {
    switch(*cur) {
    case '"':
      wabi_next(fd, cur);
      leaf->length = size | WABI_TAG_BIN_LEAF;
      return (wabi_binary) leaf;
      /* case '\\': */
      /*   escape; */
    default:
      *blob= *cur;
      wabi_next(fd, cur);
      blob++;
      size++;
    }
  }
  wabi_binary more = wabi_read_binary(vm, fd, cur);
  if(vm->errno) return NULL;
  return wabi_binary_concat_raw(&(vm->store), (wabi_binary) leaf, more);
}


static wabi_symbol
wabi_read_symbol(wabi_vm vm,
                 FILE *fd,
                 char* cur)
{
  char *blob;
  size_t size;
  wabi_binary_leaf leaf = (wabi_binary_leaf) wabi_binary_new(vm, WABI_READ_WORD_SIZE);

  if(vm->errno) return NULL;
  blob = (char*) leaf->data_ptr;
  size = 0;
  while(size < WABI_READ_WORD_SIZE) {
    switch(*cur) {
    case ' ':
    case '\n':
    case '\t':
    case '\f':
    case '\r':
    case ')':
    case ']':
    case '}':
    case '"':
    case ':':
      leaf->length = size | WABI_TAG_BIN_LEAF;
      return (wabi_symbol) wabi_intern(vm, (wabi_val) leaf);
    case EOF:
      vm->errno = WABI_ERROR_SYMBOL_TOO_LONG;
      return NULL;
    default:
      *blob= *cur;
      wabi_next(fd, cur);
      blob++;
      size++;
    }
  }
  vm->errno = WABI_ERROR_SYMBOL_TOO_LONG;
  return NULL;
}


static wabi_val
wabi_read_number(wabi_vm vm,
                 FILE *fd,
                 char* cur)
{
  wabi_word_t n = 0;
  while(*cur >= '0' && *cur <= '9') {
    n = n * 10 + (*cur - '0');
    wabi_next(fd, cur);
  }
  return wabi_smallint(vm, n);
}


static wabi_val
wabi_read_rec(wabi_vm vm,
              FILE *fd,
              char* cur)
{
  wabi_whitespace(fd, cur);
  switch(*cur) {
  case '(':
    wabi_next(fd, cur);
    return wabi_read_list(vm, fd, cur);
  case '"':
    wabi_next(fd, cur);
    return (wabi_val) wabi_read_binary(vm, fd, cur);
  /* case '{': */
  /*   return wabi_read_map(vm, fd); */
  /* case '[': */
  /*   return wabi_read_sequence(vm, fd); */
  default:
    if(*cur >= '0' && *cur <= '9') {
      return wabi_read_number(vm, fd, cur);
    }
    else {
      return wabi_read_symbol(vm, fd, cur);
    }
  }
}


wabi_val
wabi_read_raw(wabi_vm vm,
              FILE *fd)
{
  char cur;
  wabi_next(fd, &cur);
  wabi_val res = wabi_read_rec(vm, fd, &cur);
  return res;
}
