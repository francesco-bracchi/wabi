/**
 * Slow and mostly incorrect reader
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
#include "wabi_list.h"
#include "wabi_symbol.h"
#include "wabi_atom.h"
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
  switch (c) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
  case '\f':
    return 1;
  default:
    return 0;
  }
}

static inline int
wabi_reader_is_num(char c)
{
  return c >= '0' && c <= '9';
}

static inline void
wabi_reader_ws(char** c)
{
  while(wabi_reader_is_ws(**c))
    (*c)++;
}

wabi_val
wabi_reader_read_val(const wabi_vm vm, char** c);


static inline wabi_val
wabi_reader_read_list(const wabi_vm vm, char** c)
{
  wabi_val a, d;
  wabi_reader_ws(c);
  if(**c == ')') {
    (*c)++;
    return vm->emp;
  }
  a = wabi_reader_read_val(vm, c);
  if(vm->ert) return NULL;
  wabi_reader_ws(c);
  if(**c == ')') {
    (*c)++;
    d = vm->emp;
  }
  else if(**c == '.') {
    (*c)++;
    wabi_reader_ws(c);
    d = wabi_reader_read_val(vm, c);
    if(vm->ert) return NULL;
    wabi_reader_ws(c);
    if (**c != ')') {
      vm->ert = wabi_error_read;
      return NULL;
    }
    (*c)++;
  }
  else {
    wabi_reader_ws(c);
    d = wabi_reader_read_list(vm, c);
    if(vm->ert) return NULL;
  }
  return (wabi_val) wabi_cons(vm, a, d);
}


static inline wabi_val
wabi_reader_read_map(const wabi_vm vm, char** c)
{
  wabi_val a, d;
  wabi_reader_ws(c);
  if(**c == '}') {
    (*c)++;
    return vm->emp;
  }
  else {
    a = wabi_reader_read_val(vm, c);
    if(vm->ert) return NULL;
    wabi_reader_ws(c);
    d = wabi_reader_read_map(vm, c);
    if(vm->ert) return NULL;
    return (wabi_val) wabi_cons(vm, a, d);
  }
}


static inline wabi_val
wabi_reader_read_vector(const wabi_vm vm, char** c)
{
  wabi_val a, d;
  wabi_reader_ws(c);
  if(**c == ']') {
    (*c)++;
    return vm->emp;
  }
  else {
    a = wabi_reader_read_val(vm, c);
    if(vm->ert) return NULL;
    wabi_reader_ws(c);
    d = wabi_reader_read_vector(vm, c);
    if(vm->ert) return NULL;
    return (wabi_val) wabi_cons(vm, a, d);
  }
}


static inline wabi_val
wabi_reader_read_quote(const wabi_vm vm, char** c)
{
  wabi_val a, x, bin, sym;
  wabi_reader_ws(c);
  a = wabi_reader_read_val(vm, c);
  if(vm->ert) return NULL;
  wabi_reader_ws(c);
  x = (wabi_val) wabi_cons(vm, a, vm->emp);
  if(vm->ert) return NULL;

  bin = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "q");
  if(vm->ert) return NULL;
  sym = (wabi_val) wabi_symbol_new(vm, bin);
  if(vm->ert) return NULL;

  return (wabi_val) wabi_cons(vm, sym, x);
}


static inline wabi_val
wabi_reader_read_afn(wabi_vm vm, char** c)
{
  wabi_val a, x, bin, afn;
  wabi_reader_ws(c);
  a = wabi_reader_read_val(vm, c);
  if(vm->ert) return NULL;
  wabi_reader_ws(c);
  x = (wabi_val) wabi_cons(vm, a, vm->emp);
  if(vm->ert) return NULL;
  bin = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "afn");
  if(vm->ert) return NULL;
  afn = wabi_symbol_new(vm, bin);
  if(vm->ert) return NULL;
  return (wabi_val) wabi_cons(vm, afn, x);
}



static inline wabi_val
wabi_reader_read_num(wabi_vm vm, char** c)
{
  long num;
  num = 0;
  do {
    num = 10 * num + (**c - '0');
    (*c)++;
    while(**c == '_') (*c)++;
  } while(wabi_reader_is_num(**c));

  return wabi_fixnum_new(vm, num);
}


static inline wabi_val
wabi_reader_read_string(wabi_vm vm, char** c)
{
  char buff[1024], *bptr;
  wabi_val res;
  bptr = (char*) &buff;
  while(**c != '"' && **c != '\0' && (bptr - buff) < 1023) {
    if(**c == '\\') (*c)++;
    *bptr = **c;
    bptr++;
    (*c)++;
  }
  *bptr = '\0';
  res = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, (char*) &buff);
  (*c)++;
  return res;
}


static inline wabi_val
wabi_reader_read_atom(wabi_vm vm, char** c)
{
  char *buff, *bptr;
  wabi_val res;
  buff = malloc(1024);
  bptr = buff;
  while(!wabi_reader_is_ws(**c) && (**c != ')') && (**c != '}') && (**c != '(') && (**c != '"') && (**c != ';') && (**c != ':')) {
    if(**c == '\\') (*c)++;
    *bptr = **c;
    bptr++;
    (*c)++;
  }
  *bptr = '\0';
  res = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff);
  free(buff);
  if(! res) return NULL;
  return (wabi_val) wabi_atom_new(vm, res);
}

static inline wabi_val
wabi_reader_read_symbol(wabi_vm vm, char** c)
{
  char *buff, *bptr;
  wabi_val res;
  buff = malloc(1024);
  bptr = buff;
  while(!wabi_reader_is_ws(**c) && (**c != ')') && (**c != '}') && (**c != '(') && (**c != '"') && (**c != ';') && (**c != ':')) {
    if(**c == '\\') (*c)++;
    *bptr = **c;
    bptr++;
    (*c)++;
  }
  *bptr = '\0';
  res = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, buff);
  free(buff);
  if(! res) return NULL;
  return (wabi_val) wabi_symbol_new(vm, res);
}

wabi_val
wabi_reader_neg(wabi_val num)
{
  long lnum;

  lnum = - WABI_CAST_INT64(num);
  *num = WABI_WORD_VAL(lnum);
  WABI_SET_TAG(num, wabi_tag_fixnum);
  return num;
}

wabi_val
wabi_reader_read_val(const wabi_vm vm, char** c)
{
  wabi_val bin, sym;
  wabi_reader_ws(c);

  switch(**c) {
  case '\'':
    (*c)++;
    return wabi_reader_read_quote(vm, c);

  case '#':
    (*c)++;
    return wabi_reader_read_afn(vm, c);

  case '(':
    (*c)++;
    return wabi_reader_read_list(vm, c);

  case  '{':
    (*c)++;
    bin = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "hmap");
    if(vm->ert) return NULL;
    sym = (wabi_val) wabi_symbol_new(vm, bin);
    if(vm->ert) return NULL;
    return (wabi_val) wabi_cons(vm, sym, wabi_reader_read_map(vm, c));

  case  '[':
    (*c)++;
    bin = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "vec");
    if(vm->ert) return NULL;
    sym = (wabi_val) wabi_symbol_new(vm, bin);
    if(vm->ert) return NULL;
    return (wabi_val) wabi_cons(vm, sym, wabi_reader_read_vector(vm, c));

  case  ';':
    (*c)++;
    bin = (wabi_val) wabi_binary_leaf_new_from_cstring(vm, "cmt");
    if(vm->ert) return NULL;
    sym = (wabi_val) wabi_symbol_new(vm, bin);
    if(vm->ert) return NULL;
    return (wabi_val) wabi_cons(vm, sym, wabi_reader_read_val(vm, c));

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return wabi_reader_read_num(vm, c);

  case '-':
    if (wabi_reader_is_num(*(*c + 1))) {
      (*c)++;
      return wabi_reader_neg(wabi_reader_read_num(vm, c));
    }
    return wabi_reader_read_symbol(vm, c);

  case '"':
    (*c)++;
    return wabi_reader_read_string(vm, c);

  case '_':
    (*c)++;
    return vm->ign;

  case ':':
    (*c)++;
    return wabi_reader_read_atom(vm, c);

  case '\0':
    return NULL;
  default:
    return wabi_reader_read_symbol(vm, c);
  }
}

wabi_val
wabi_reader_read(const wabi_vm vm, char* c) {
  return wabi_reader_read_val(vm, &c);
}


inline static wabi_val
wabi_reader_reverse(const wabi_vm vm, wabi_val es)
{
  wabi_val xs;

  xs = vm->emp;
  while(wabi_is_pair(es)) {
    xs = (wabi_val) wabi_cons(vm, wabi_car((wabi_pair) es), xs);
    if(vm->ert) return NULL;
    es = wabi_cdr((wabi_pair) es);
  }
  if(! wabi_atom_is_empty(vm, es)) {
    vm->ert = wabi_error_read;
  }
  return xs;
}

wabi_val
wabi_reader_read_all(const wabi_vm vm, char* c)
{
  wabi_val ex, exs;
  exs = vm->emp;

  for(;;) {
    ex = wabi_reader_read_val(vm, &c);
    if(vm->ert) return NULL;
    if(! ex) break;

    exs = (wabi_val) wabi_cons(vm, ex, exs);
    if(vm->ert) return NULL;
  }
  return wabi_reader_reverse(vm, exs);
}
