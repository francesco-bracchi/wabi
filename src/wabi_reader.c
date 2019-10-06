/**
 * Fast and mostly incorrect reader
 */

#define wabi_reader_c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_number.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_symbol.h"
#include "wabi_reader.h"

#define IS_WS(c) ((c) == ' ' || (c) == '\n' || (c) == '\t')


typedef enum wabi_reader_state_enum
  {
   wabi_reader_unkown = 0,
   wabi_reader_string = 1,
   wabi_reader_number = 2,
  } wabi_reader_state;


typedef struct wabi_reader_struct {
  wabi_vm vm;
  wabi_val stack;
  wabi_reader_state state;
  long number;
  wabi_binary bin;
  unsigned long string_pos;
  unsigned long string_size;
} wabi_reader_t;


typedef wabi_reader_t* wabi_reader;


static inline int
wabi_reader_is_ws(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

static inline int
wabi_reader_is_num(char c)
{
  return c >= '0' && c <= '9';
}

static inline void
wabi_reader_ws(char** c)
{
  for(;;) {
    while(wabi_reader_is_ws(**c))
      (*c)++;

    if(**c == ';')
      do {
        (*c)++;
      } while(**c != '\n' && **c != '\0');
    else
      return;
  }
}

wabi_val
wabi_reader_read_val(wabi_vm vm, char** c);


static inline wabi_val
wabi_reader_read_list(wabi_vm vm, char** c)
{
  wabi_val a, d;
  wabi_reader_ws(c);
  a = wabi_reader_read_val(vm, c);
  if(! a) return NULL;
  wabi_reader_ws(c);
  if(**c == ')') {
    (*c)++;
    if(!wabi_vm_has_rooms(vm, 1)) return NULL;

    d = wabi_vm_alloc(vm, 1);
    *d = wabi_val_nil;
  }
  else if(**c == '.') {
    (*c)++;
    wabi_reader_ws(c);
    d = wabi_reader_read_val(vm, c);
    if(! d) return NULL;
    wabi_reader_ws(c);
    if(**c != ')') return NULL;
    (*c)++;
  }
  else {
    wabi_reader_ws(c);
    d = wabi_reader_read_list(vm, c);
    if(! d) return NULL;
  }
  return (wabi_val) wabi_cons(vm, a, d);
}


static inline wabi_val
wabi_reader_read_num(wabi_vm vm, char** c)
{
  long num;
  num = 0;
  do {
    num = 10 * num + (**c - '0');
    (*c)++;
  } while(wabi_reader_is_num(**c));

  return wabi_fixnum_new(vm, num);
}


static inline wabi_val
wabi_reader_read_string(wabi_vm vm, char** c)
{
  char *buff, *bptr;
  wabi_val res;
  buff = malloc(1024);
  bptr = buff;
  while(**c != '"' && **c != '\0' && (bptr - buff) < 1023) {
    if(**c == '\\') (*c)++;
    *bptr = **c;
    bptr++;
    (*c)++;
  }
  *bptr = '\0';
  res = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff);
  (*c)++;
  free(buff);
  return res;
}


static inline wabi_val
wabi_reader_read_symbol(wabi_vm vm, char** c)
{
  char *buff, *bptr;
  wabi_val res;
  buff = malloc(1024);
  bptr = buff;
  while(!wabi_reader_is_ws(**c) && (**c != ')') && (**c != '(') && (**c != '"') && (**c != ';')) {
    if(**c == '\\') (*c)++;
    *bptr = **c;
    bptr++;
    (*c)++;
  }
  *bptr = '\0';
  res = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff);
  free(buff);
  if(! res) return NULL;
  return wabi_symbol_new(vm, res);
}

static inline wabi_val
wabi_reader_read_ignore(wabi_vm vm) {
  wabi_val res;
  if(wabi_vm_has_rooms(vm, 1)) {
    res = wabi_vm_alloc(vm, 1);
    *res = wabi_val_ignore;
    return res;
  }
  vm->errno = wabi_error_nomem;
  return NULL;
}


wabi_val
wabi_reader_read_val(wabi_vm vm, char** c)
{
  wabi_reader_ws(c);
  if(**c == '(') {
    (*c)++;
    return wabi_reader_read_list(vm, c);
  }
  if(wabi_reader_is_num(**c)) {
    return wabi_reader_read_num(vm, c);
  }
  if(**c == '"') {
    (*c)++;
    return wabi_reader_read_string(vm, c);
  }
  if(**c == '_') {
    (*c)++;
    return wabi_reader_read_ignore(vm);
  }
  if(**c == '\0') {
    return NULL;
  }
  return wabi_reader_read_symbol(vm, c);
}

wabi_val
wabi_reader_read(wabi_vm vm, char* c) {
  return wabi_reader_read_val(vm, &c);
}
