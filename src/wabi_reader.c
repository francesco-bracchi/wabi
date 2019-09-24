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
  wabi_val nil;
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

static inline char*
wabi_reader_ws(char* c)
{
  while(wabi_reader_is_ws(*c)) c++;
  return c;
}

static inline wabi_val
wabi_reader_reverse(wabi_vm vm,
                    wabi_val done)
{
  wabi_val res;
  res = wabi_nil(vm);
  while(*done != wabi_val_nil) {
    res = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) done), res);
    done = wabi_cdr((wabi_pair) done);
  }
  return res;
}

wabi_val
wabi_reader_read(wabi_vm vm, char* c)
{
  wabi_pair stack;
  wabi_val nil, sub;
  long num;
  char *buff, *bptr;

  nil = wabi_nil(vm);
  stack = wabi_cons(vm, nil, nil);

  do {
    c = wabi_reader_ws(c);
    if(*c == '(') {
      c++;
      stack = wabi_cons(vm, nil, (wabi_val) stack);
      continue;
    }
    if(*c == ')') {
      c++;
      sub = wabi_reader_reverse(vm, wabi_car(stack));
      stack = (wabi_pair) wabi_cdr(stack);
      stack = wabi_cons(vm, (wabi_val) wabi_cons(vm, wabi_car(stack), sub), wabi_cdr(stack));
      continue;
    }
    if(wabi_reader_is_num(*c)) {
      num = 0;
      do {
        num = 10 * num + (*c - '0');
        c++;
      } while(wabi_reader_is_num(*c));
      stack = wabi_cons(vm, (wabi_val) wabi_cons(vm, (wabi_val) wabi_fixnum_new(vm, num), wabi_car(stack)), wabi_cdr(stack));
    }
    if(*c == '"') {
      c++;
      buff = malloc(1024);
      bptr = buff;
      do {
        if(*c == '\\') c++;
        *bptr = *c;
        bptr++;
        c++;
      } while(*c != '"' && bptr - buff < 1023);
      c++;
      *bptr = '\0';
      stack = wabi_cons(vm, (wabi_val) wabi_cons(vm, (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff), wabi_car(stack)), wabi_cdr(stack));
      free(buff);
    }
    if (*c >= 'a') {
      buff = malloc(1024);
      bptr = buff;
      do {
        *bptr = *c;
        bptr++;
        c++;
      } while(!wabi_reader_is_ws(*c));
      c++;
      *bptr = '\0';
      stack = wabi_cons(vm, (wabi_val) wabi_cons(vm, (wabi_val) wabi_symbol_new(vm, (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff)), wabi_car(stack)), wabi_cdr(stack));
      free(buff);
    }
  } while(*c != '\0');
  return wabi_car((wabi_pair) wabi_reader_reverse(vm, (wabi_val) stack));
}
